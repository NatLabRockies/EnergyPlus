// This file gathers the operations that do require Admin privileges

function Component()
{
  Component.prototype.createOperations = function()
  {
    // call default implementation
    component.createOperations();

    // ... add custom operations

    var kernel = systemInfo.kernelType;
    if(( kernel == "darwin" ) || ( kernel == "linux")) {

      // Symlinks: require admin privileges
      var linktarget = "/usr/local/bin";
      component.addElevatedOperation("Execute", "ln", "-sf", "@TargetDir@/PreProcess/GrndTempCalc/Basement", linktarget,
                                     "UNDOEXECUTE", "rm", linktarget + "/Basement");
      component.addElevatedOperation("Execute", "ln", "-sf", "@TargetDir@/PreProcess/GrndTempCalc/BasementGHT.idd", linktarget,
                                     "UNDOEXECUTE", "rm", linktarget + "/BasementGHT.idd");
      component.addElevatedOperation("Execute", "ln", "-sf", "@TargetDir@/PostProcess/convertESOMTRpgm/convertESOMTR", linktarget,
                                     "UNDOEXECUTE", "rm", linktarget + "/convertESOMTR");
      component.addElevatedOperation("Execute", "ln", "-sf", "@TargetDir@/energyplus", linktarget,
                                     "UNDOEXECUTE", "rm", linktarget + "/energyplus");
      component.addElevatedOperation("Execute", "ln", "-sf", "@TargetDir@/Energy+.idd", linktarget,
                                     "UNDOEXECUTE", "rm", linktarget + "/Energy+.idd");
      component.addElevatedOperation("Execute", "ln", "-sf", "@TargetDir@/Energy+.schema.epJSON", linktarget,
                                     "UNDOEXECUTE", "rm", linktarget + "/Energy+.schema.epJSON");
      component.addElevatedOperation("Execute", "ln", "-sf", "@TargetDir@/EPMacro", linktarget,
                                     "UNDOEXECUTE", "rm", linktarget + "/EPMacro");
      component.addElevatedOperation("Execute", "ln", "-sf", "@TargetDir@/ExpandObjects", linktarget,
                                     "UNDOEXECUTE", "rm", linktarget + "/ExpandObjects");
      component.addElevatedOperation("Execute", "ln", "-sf", "@TargetDir@/PostProcess/HVAC-Diagram", linktarget,
                                     "UNDOEXECUTE", "rm", linktarget + "/HVAC-Diagram");
      component.addElevatedOperation("Execute", "ln", "-sf", "@TargetDir@/PreProcess/ParametricPreprocessor/ParametricPreprocessor", linktarget,
                                     "UNDOEXECUTE", "rm", linktarget + "/ParametricPreprocessor");
      component.addElevatedOperation("Execute", "ln", "-sf", "@TargetDir@/PostProcess/ReadVarsESO", linktarget,
                                     "UNDOEXECUTE", "rm", linktarget + "/ReadVarsESO");
      component.addElevatedOperation("Execute", "ln", "-sf", "@TargetDir@/runenergyplus", linktarget,
                                     "UNDOEXECUTE", "rm", linktarget + "/runenergyplus");
      component.addElevatedOperation("Execute", "ln", "-sf", "@TargetDir@/runepmacro", linktarget,
                                     "UNDOEXECUTE", "rm", linktarget + "/runepmacro");
      component.addElevatedOperation("Execute", "ln", "-sf", "@TargetDir@/runreadvars", linktarget,
                                     "UNDOEXECUTE", "rm", linktarget + "/runreadvars");
      component.addElevatedOperation("Execute", "ln", "-sf", "@TargetDir@/PreProcess/GrndTempCalc/Slab", linktarget,
                                     "UNDOEXECUTE", "rm", linktarget + "/Slab");
      component.addElevatedOperation("Execute", "ln", "-sf", "@TargetDir@/PreProcess/GrndTempCalc/SlabGHT.idd", linktarget,
                                     "UNDOEXECUTE", "rm", linktarget + "/SlabGHT.idd");

      if( kernel == "linux") {
        // Historical: TODO: is this needed really?
        component.addElevatedOperation("Execute", "ln", "-sf", "@TargetDir@/energyplus", linktarget + "/EnergyPlus",
                                       "UNDOEXECUTE", "rm", linktarget + "/EnergyPlus")

        component.addElevatedOperation("Execute", "ln", "-sf", "@TargetDir@/PreProcess/IDFVersionUpdater/IDFVersionUpdater", linktarget,
                                       "UNDOEXECUTE", "rm", linktarget + "/IDFVersionUpdater");

        component.addElevatedOperation("Execute", "ln", "-sf", "@TargetDir@/PreProcess/FMUParser/parser", linktarget,
                                       "UNDOEXECUTE", "rm", linktarget + "/parser");
      }

      // TODO: we should perhaps create symlinks to
      // IDFVersionUpdater and and FMUparser on mac too, eg:

      // man page:
      linktarget = "/usr/local/share/man/man1";
      component.addElevatedOperation("Execute", "mkdir", "-p", linktarget);

      component.addElevatedOperation("Execute", "ln", "-sf", "@TargetDir@/energyplus.1", linktarget + "/energyplus.1",
                                     "UNDOEXECUTE", "rm", linktarget + "/energyplus.1");

    }
  }
}
