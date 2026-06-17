#include <cassert>
#include <iostream>
#include <dlfcn.h>
int main() {
    const char *dlsym_error;
    std::cout << "Opening eplus shared library...\n";
    void* handle = dlopen("{EPLUS_INSTALL_NO_SLASH}{LIB_FILE_NAME}", RTLD_LAZY);
    if (!handle) {
        std::cerr << "Cannot open library: \n";
        return 1;
    }
    dlerror(); // resets errors
    //
    std::cout << "Getting a new state instance...\n";
    typedef void *(*fNewState)();
    auto stateNew = (fNewState) dlsym(handle, "stateNew");
    dlsym_error = dlerror();
    if (dlsym_error) {
        std::cerr << "Cannot load symbol stateNew\n";
        dlclose(handle);
        return 1;
    }
    auto state = stateNew();
    dlsym_error = dlerror();
    if (dlsym_error) {
        std::cerr << "Cannot instantiate a new state from stateNew\n";
        dlclose(handle);
        return 1;
    }
    //
    std::cout << "Calling to initialize...\n";
    typedef void (*init_t)(void *);
    auto init = (init_t) dlsym(handle, "initializeFunctionalAPI");
    dlsym_error = dlerror();
    if (dlsym_error) {
        std::cerr << "Cannot load symbol 'initializeFunctionalAPI': \n";
        dlclose(handle);
        return 1;
    }
    init(state);
    dlsym_error = dlerror();
    if (dlsym_error) {
        std::cerr << "Could not call initialize function \n";
        dlclose(handle);
        return 1;
    }
    //
    std::cout << "Getting a new Glycol instance...\n";
    typedef void* (*newGly)(void *, const char *);
    auto thisNewGly = (newGly) dlsym(handle, "glycolNew");
    dlsym_error = dlerror();
    if (dlsym_error) {
        std::cerr << "Cannot load symbol 'glycolNew': \n";
        dlclose(handle);
        return 1;
    }
    auto glycolInstance = thisNewGly(state, "water");
    dlsym_error = dlerror();
    if (dlsym_error) {
        std::cerr << "Cannot get a new glycol instance via glycolNew': \n";
        dlclose(handle);
        return 1;
    }
    //
    std::cout << "Calculating Cp at T = 25C...\n";
    typedef double(*cp)(void *, void *, double);
    auto glycolCp = (cp) dlsym(handle, "glycolSpecificHeat");
    dlsym_error = dlerror();
    if (dlsym_error) {
        std::cerr << "Cannot load symbol 'glycolSpecificHeat': \n";
        dlclose(handle);
        return 1;
    }
    auto cpValue = glycolCp(state, glycolInstance, 25);
    dlsym_error = dlerror();
    if (dlsym_error) {
        std::cerr << "Cannot calculate Cp with glycolSpecificHeat': \n";
        dlclose(handle);
        return 1;
    }
    //
    std::cout << "Calculated Cp = " << cpValue << "\n";
    assert(cpValue > 4150);
    assert(cpValue < 4200);
    std::cout << "Closing library...\n";
    dlclose(handle);
}
