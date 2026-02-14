// Regular install commands to be performed
function Component()
{
  Component.prototype.createOperations = function()
  {
    // call default implementation
    component.createOperations();

    // ... add custom operations

    var kernel = systemInfo.kernelType;
    if( kernel == "darwin" ) {

      // Chmod +x for apps
      component.addOperation("Execute", "chmod", "+x", "@TargetDir@/PreProcess/IDFVersionUpdater/IDFVersionUpdater.app/Contents/MacOS/IDFVersionUpdater");
      component.addOperation("Execute", "chmod", "+x", "@TargetDir@/PreProcess/EP-Launch-Lite.app/Contents/MacOS/EP-Launch-Lite");
      component.addOperation("Execute", "chmod", "+x", "@TargetDir@/PreProcess/EP-Launch-Lite.app/Contents/MacOS/python");

      // Not sure necessary so not doing it yet
      // component.addOperation("Execute", "chmod", "-R", "a+w", "@TargetDir@");
      // component.addOperation("Execute", "chmod", "-R", "a+w", "@TargetDir@/PreProcess/IDFVersionUpdater/IDFVersionUpdater.app/Contents/MacOS/");
      // component.addOperation("Execute", "chmod", "-R", "a+w", "@TargetDir@/PreProcess/IDFVersionUpdater/");

    }

    // On Windows
    if( kernel == "winnt" ) {

      // Create Shortcuts in the Windows Start Menu: done separately (optional)

      // Note: Associate file types: done separately (optional)

      // Here's what stuff gets weird. We need to write stuff to the registry apparently for EP-Launch
      // In the registry under KEY_CURRENT_USER\Software\VB and VBA Program Settings\EP-Launch\UpdateCheck:
      // Name,       Type,   Data
      // AutoCheck,  REG_SZ, True
      // CheckURL,   REG_SZ, http://nrel.github.io/EnergyPlus/epupdate.htm
      // LastAnchor, REG_SZ, #9.1.0-mqjdfsiojf

      // REG ADD KeyName /v ValueName, /d Data, /f = force overwrite
      var reg = installer.environmentVariable("SystemRoot").replace(/\//g, '\\') + "\\System32\\reg.exe";

      var keyName = "HKEY_CURRENT_USER\\Software\\VB and VBA Program Settings\\EP-Launch\\UpdateCheck";

      // Note by passing arguments separately to the (reg) command in addElevatedOperation (supports up to 10 args),
      // Qt will escape each argument, so do not include double quote (eg: "\"AutoCheck\"") or they'll be interpreted literally
      // Mind the "/f" flag which avoids displaying a [Yes/No] prompt that you
      // can't answer and making the installer freeze
      var valueName = "AutoCheck";
      var data = "True";
      component.addOperation("Execute", reg, "ADD", keyName, "/v", valueName, "/d", data, "/f");

      var valueName = "CheckURL";
      var data = "http://nrel.github.io/EnergyPlus/epupdate.htm";
      component.addOperation("Execute", reg, "ADD", keyName, "/v", valueName, "/d", data, "/f");

      var valueName = "LastAnchor";
      var data = "#@Version@";
      component.addOperation("Execute", reg, "ADD", keyName, "/v", valueName, "/d", data, "/f");

      // Delete the entire keyName upon uninstallation
      // Upon discussion, we do not want to delete this key where EP-Launch
      // stores its settings (Options) such as text editor of choice etc
      // So they remain available for other installations or next installation
      // var keyName = "HKEY_CURRENT_USER\\Software\\VB and VBA Program Settings\\EP-Launch";
      // component.addOperation("Execute", "cmd", "/C", "echo Set up uninstall operation to delete EP-Launch registry keys", "UNDOEXECUTE", reg, "DELETE", keyName, "/f");

      // And weirder still, to copy and register DLLs: done separately
    }
  }
}
