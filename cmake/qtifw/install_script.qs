/* QtIFW Installer script for EnergyPlus

Allows performing silent installs on all platforms.

## Installation

NOTE: By running a silent install, you agree to the EnergyPlus License Agreement

Usage:

```
# You can get the latest version of this script by doing something similar to:
wget https://raw.githubusercontent.com/NatLabRockies/EnergyPlus/develop/cmake/qtifw/install_script.qs
# Windows Powershell
wget https://raw.githubusercontent.com/NatLabRockies/EnergyPlus/develop/cmake/qtifw/install_script.qs -OutFile install_script.qs

# Linux
sudo ./EnergyPlus-9.2.0-0e6e9c08a0-Linux-x86_64.run --verbose --platform minimal --script /path/to/install_script.qs

# Mac: `--plaftorm minimal` appears to produce a segmentation fault
open EnergyPlus-9.2.0-0e6e9c08a0-Darwin-x86_64.dmg
sudo /Volumes/EnergyPlus-9.2.0-0e6e9c08a0-Darwin-x86_64/EnergyPlus-9.2.0-0e6e9c08a0-Darwin-x86_64.app/Contents/MacOS/EnergyPlus-9.2.0-0e6e9c08a0-Darwin-x86_64 --verbose --script /path/to/install_script.qs

# Windows: open cmd.exe as admin: `--platform minimal` appears unsupported (but we use gui.setSilent(true) so nothing will pop up)
EnergyPlus-9.2.0-0e6e9c08a0-Windows-x86_64.exe --verbose --script /path/to/install_script.qs
```

You can also customize the install directory by passing `TargetDir`
If you don't, it defaults to:
* Linux: `/usr/local/EnergyPlus-9-2-0`
* Mac: `/Applications/EnergyPlus-9-2-0`
* Windows: `C:\EnergyPlusV9-2-0`

```
 sudo ./EnergyPlus-9.2.0-0e6e9c08a0-Linux-x86_64.run --verbose --platform minimal --script /path/to/install_script.qs TargetDir=/usr/local/Eplus
```

By default, all components will be installed.
We also built-in a way to unselect components via the command line by passing
`<ComponentName>=false` (case matters, has to be `false` exactly)

Unix:
```
 sudo ./EnergyPlus-9.2.0-0e6e9c08a0-Linux-x86_64.run --verbose --platform minimal --script /path/to/install_script.qs Documentation=false ExampleFiles=false WeatherData=false Datasets=false Symlinks=false
```

Windows: Open cmd.exe as admin

```
EnergyPlus-9.2.0-0e6e9c08a0-Windows-x86_64.exe --verbose --script /path/to/install_script.qs Documentation=false ExampleFiles=false WeatherData=false Datasets=false CreateStartMenu=false RegisterFileType=false
```

On Windows only, if CreateStartMenu is true (default), there is a built-in option `AllUsers=true`
which will in particular create the shortcuts in the All Users start menu (C:\ProgramData\Microsoft\Windows\Start Menu\Programs)
which is useful if you use SCCM (or psexec) to run the commands as you are LOCAL SYSTEM account when you do and these shortcuts won't work for the regular user

```
EnergyPlus-9.2.0-0e6e9c08a0-Windows-x86_64.exe --verbose --script /path/to/install_script.qs AllUsers=true
```

-------------------------------------------------------------------------------

## Uninstallation

NOTE: You can also pass this same script to the maintenancetool for a silent
complete uninstall

```
# Linux
sudo /usr/local/EnergyPlus-9-2-0/maintenancetool --verbose --plaftorm minimal --script /path/to/install_script.qs

# Mac: `--platform minimal` appears to produce a segfault right now
sudo /Applications/EnergyPlus-9-2-0/maintenancetool.app/Contents/MacOS/maintenancetool --verbose --script /path/to/install_script.qs

# Windows: `--platform minimal` appears unsupported at the moment
C:\EnergyPlusV9-2-0\maintenancetool.exe --verbose --script /path/to/install_script.qs
```

**/


function Controller() {
  installer.autoRejectMessageBoxes();

  // silent install is not an option until QtIFW v3.0.1
  gui.setSilent(true);

  installer.setMessageBoxAutomaticAnswer("OverwriteTargetDirectory",
                                         QMessageBox.Yes);

  installer.installationFinished.connect(function() {
    gui.clickButton(buttons.NextButton);
  });
  // Uninstaller
  installer.uninstallationFinished.connect(function() {
    gui.clickButton(buttons.NextButton);
  });
}

function logCurrentPage() {
    var pageName = gui.currentPageWidget().objectName;
    var pagePrettyTitle = gui.currentPageWidget().title;
    console.log("At page: " + pageName + " ('" + pagePrettyTitle + "')");
}

// NOT USED
/*
 *Controller.prototype.WelcomePageCallback = function() {
 *  console.log("---- WELCOME PAGE");
 *  logCurrentPage();
 *  // click delay because the next button is initially disabled for ~1s
 *  gui.clickButton(buttons.NextButton, 3000);
 *};
 *
 *Controller.prototype.CredentialsPageCallback = function() {
 *  console.log("---- CREDENTIAL PAGE");
 *  logCurrentPage();
 *  gui.clickButton(buttons.NextButton);
 *};
 */

Controller.prototype.IntroductionPageCallback = function() {
  console.log("---- INTRODUCTION PAGE");
  logCurrentPage();
  gui.clickButton(buttons.NextButton);
};

Controller.prototype.TargetDirectoryPageCallback = function()
{
  console.log("---- TARGET DIRECTORY PAGE");
  logCurrentPage();

  console.log("User-suplied TargetDir: " + installer.value("TargetDir"));

  // gui.currentPageWidget().TargetDirectoryLineEdit.setText(installer.value("harcoded/path/EnergyPlus");
  console.log("Target dir: " +
              gui.currentPageWidget().TargetDirectoryLineEdit.text);

  gui.clickButton(buttons.NextButton);
};

Controller.prototype.ComponentSelectionPageCallback = function() {
  console.log("---- COMPONENT SELECTION PAGE");
  logCurrentPage();

  var widget = gui.currentPageWidget();

  var components = installer.components();
  console.log("There are " + components.length + " available components.");
  console.log("Components selection for installation:");
  components.forEach(component => {
    var compName = component.name;
    var installStatus = "Yes";

    // Get command line args passed
    var thisCompFlag = installer.value(compName);

    if (thisCompFlag === "false") {
      // Protect against trying to deselect something required
      if (["Unspecified", "Licenses"].indexOf(compName) >= 0) {
        console.log("-- Component '" + compName +
                    "' is mandatory and cannot be unselected");
        installStatus = "Yes (FORCED AS REQUIRED)";
      } else if (compName === "CopyAndRegisterSystemDLLs") {
        console.log("-- Component 'CopyAndRegisterSystemDLLs' is highly recommended on Windows, and it will not overwrite any existing DLLs so it should not have side effects");
        // Allow unselect anyways
        installStatus = "No (use at your own risk)";
        widget.deselectComponent(compName);
      } else if (compName === "Libraries") {
        console.log("-- Component 'Libraries' is highly recommended on Windows, it will include msvc runtime, and also Windows Universal CRT libraries (needed for shipping to Windows 7/8).");
        // Allow unselect anyways
        installStatus = "No (use at your own risk)";
        widget.deselectComponent(compName);
      } else {
        installStatus = "No";
        widget.deselectComponent(compName);
      }
    }
    console.log("* " + compName + ": " + installStatus);
  });

  // widget.deselectAll();

  // All Plaftorms: "Documentation", "Datasets", "ExampleFiles",
  // "WeatherData"

  // Unix (Mac/Linux)
  // widget.deselectComponent("Symlinks");

  // Windows: "CreateStartMenu", "RegisterFileType",
  // "CopyAndRegisterSystemDLLs", "Libraries"


  // Linux/Mac: Packages: Symlinks
  //                      Datasets Documentation ExampleFiles Licenses Unspecified WeatherData
  // Windows:   Packages: RegisterFileType CopyAndRegisterSystemDLLs CreateStartMenu Libraries
  //                      Datasets Documentation ExampleFiles Licenses Unspecified WeatherData

  gui.clickButton(buttons.NextButton);
};

Controller.prototype.LicenseAgreementPageCallback = function () {
  console.log("---- LICENSE AGREEMENT PAGE");
  logCurrentPage();
  var widget = gui.currentPageWidget();
  // AcceptLicenseRadioButton was the previous name (up to 4.1.0), now it's AcceptLicenseCheckBox
  // In case this newer script is used for a previous installer, we try a fallback to old name
  var box = widget.AcceptLicenseCheckBox;
  if (box) {
    box.setChecked(true);
  } else {
    box = widget.AcceptLicenseRadioButton;
    if (box) {
      box.setChecked(true);
    }
  }

  gui.clickButton(buttons.NextButton);
};

Controller.prototype.StartMenuDirectoryPageCallback = function() {
  console.log("---- START MENU DIRECTORY PAGE");
  logCurrentPage();

  // You won't get in this callback if it wasn't already Windows, but let's be explicit & safe
  if (systemInfo.kernelType == "winnt") {
    // TODO: extra logging for debug for now
    console.log("installer StartMenuDir: " + installer.value("StartMenuDir"));
    console.log("Text: " + gui.currentPageWidget().StartMenuPathLineEdit.text);
    console.log("AllUsersStartMenuProgramsPath: " + installer.value("AllUsersStartMenuProgramsPath"));
    console.log("UserStartMenuProgramsPath: " + installer.value("UserStartMenuProgramsPath"));
    if (installer.value("UseAllUsersStartMenu") === "true") {
      console.log("Will use the **All** Users Start Menu at: " + installer.value("AllUsersStartMenuProgramsPath"));
    } else {
      console.log("Will use this Users' Start Menu at: " + installer.value("UserStartMenuProgramsPath"));
    }
  }
  gui.clickButton(buttons.NextButton);
};

Controller.prototype.ReadyForInstallationPageCallback = function()
{
  console.log("---- READY FOR INSTALLATION PAGE");
  logCurrentPage();
  gui.clickButton(buttons.CommitButton);
};

Controller.prototype.PerformInstallationPageCallback = function()
{
  console.log("---- PERFORM INSTALLATION PAGE");
  logCurrentPage();
  gui.clickButton(buttons.CommitButton);
};

Controller.prototype.FinishedPageCallback = function() {
  console.log("---- FINISHED PAGE");
  logCurrentPage();

  gui.clickButton(buttons.FinishButton);
};
