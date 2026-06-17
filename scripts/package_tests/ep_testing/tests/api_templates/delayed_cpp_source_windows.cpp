#include <windows.h>
#include <iostream>

int main() {
  std::cout << "Opening eplus shared library...\\n";
  HINSTANCE hInst;
  hInst = LoadLibrary("{EPLUS_INSTALL_NO_SLASH}{LIB_FILE_NAME}");
  if (!hInst) {
    std::cerr << "Cannot open library: \\n";
    return 1;
  }

  // state.h: ENERGYPLUSLIB_API EnergyPlusState stateNew();
  typedef void* EnergyPlusState;
  std::cout << "Getting stateNew address\\n";
  typedef EnergyPlusState(*STATEFUNCTYPE)();
  STATEFUNCTYPE fNewState;
  fNewState = (STATEFUNCTYPE)GetProcAddress((HINSTANCE)hInst, "stateNew");
  if (!fNewState) {
    std::cerr << "Cannot get function address stateNew \\n";
    return 1;
  }

  std::cout << "Initializating a new state from stateNew\\n";
  auto state = fNewState();

  // func.h ENERGYPLUSLIB_API void initializeFunctionalAPI(EnergyPlusState state);
  std::cout << "Getting initializeFunctionalAPI address\\n";
  typedef void (*INITFUNCTYPE)(EnergyPlusState);
  INITFUNCTYPE init;
  init = (INITFUNCTYPE)GetProcAddress((HINSTANCE)hInst, "initializeFunctionalAPI");
  if (!init) {
    std::cerr << "Cannot get function \\n";
    return 1;
  }
  std::cout << "Calling to initialize via init(state)\\n";
  init(state);
  std::cout << "Closing library\\n";
  FreeLibrary((HINSTANCE)hInst);
  return 0;
}
