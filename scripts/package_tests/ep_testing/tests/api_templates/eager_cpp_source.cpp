#include <stddef.h>
#include <stdio.h>
#include <EnergyPlus/api/state.h>
#include <EnergyPlus/api/func.h>
int main() {
    EnergyPlusState state = stateNew();
    initializeFunctionalAPI(state);
    Glycol glycol = NULL;
    glycol = glycolNew(state, "WatEr");
    for (int temp=5; temp<35; temp+=10) {
        Real64 thisTemp = (float)temp;
        Real64 specificHeat = glycolSpecificHeat(state, glycol, thisTemp);
        printf("Cp = %8.3f\\n", specificHeat);
    }
    glycolDelete(state, glycol);
    printf("Hello, world!\\n");
}
