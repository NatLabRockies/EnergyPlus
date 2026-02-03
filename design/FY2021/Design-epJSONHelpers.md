epJSON helper functions to get field values
================

**M.J. Witte, GARD Analytics, Inc.**

 - Original, May 17, 2021
 - Revision Date


## Objective ##
From issue [#7632](https://github.com/NatLabRockies/EnergyPlus/issues/7632)

It would be useful to have an input processor function that

1. Takes a field name, validates the field name, and returns the input value or it's default.
2. To validate field name, will likely have to pass in cCurrentObjectItem (object class name) as an argument.
3. If the field doesn't have a declared default, then numeric should return zero, and string should return blank.
4. The EIRWWHP test for no default and no input is unusual, but for objects that want to test this for a given field, perhaps this function could return a status flag? (0 = user input, 1 = default, -1 = no user input or default) or ???

For a field where you simply want to get the value or the default and move on, the goal would be something like:

`loc_m_HeatingCoilName = inputProcessor->getFieldValue("heating_coil_name", fields, cCurrentObjectItem, status);`

## Design ##
Here's a prototype `getInput` function using 3 new functions:

    getObjectSchemaProps(state, CurrentModuleObject)
    getRealFieldValue(state, CurrentModuleObject, objectFields, objectSchemaProps, "field_name")
    getAlphaFieldValue(state, CurrentModuleObject, objectFields, objectSchemaProps, "field_name")

    // Get the schema properties for the current object type
    InputProcessor::json objectSchemaProps;
    CurrentModuleObject = "AirflowNetwork:MultiZone:Surface:Crack";
    objectSchemaProps = state.dataInputProcessing->inputProcessor->getObjectSchemaProps(state, CurrentModuleObject);

    // Get all instances of this object type from the input file
    auto instances = state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);

    if (instances != state.dataInputProcessing->inputProcessor->epJSON.end()) {
        auto &instancesValue = instances.value();

        for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
            auto const &objectFields = instance.value();
            auto const &thisObjectName = UtilityRoutines::MakeUPPERCase(instance.key());
            state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key());
            // For incoming idf, maintain object order
            ++instanceCounter;
            itemNum = state.dataInputProcessing->inputProcessor->getIDFObjNum(state, state.dataHeatBalMgr->CurrentModuleObject, instanceCounter);

            // Get simple input fields (if blank will return default or blank or zero)
            Real64 crack(itemNum).coeff = state.dataInputProcessing->inputProcessor->getRealFieldValue(
                state, CurrentModuleObject, objectFields, objectSchemaProps, "air_mass_flow_coefficient_at_reference_conditions");

            std::string crack(itemNum).referenceCrackConditionsName = state.dataInputProcessing->inputProcessor->getAlphaFieldValue(
                state, CurrentModuleObject, objectFields, objectSchemaProps, "reference_crack_conditions");

        }
    }

Here's a prototype section for retrieving extensible fields:
For this object, the extensible group is named "managers".
The extensible group name and field names can be found in the schema (Energy+.schema.epJSON).

                auto extensibles = objectFields.find("managers");
                auto const &extensionSchemaProps = objectSchemaProps["managers"]["items"]["properties"];
                if (extensibles != objectFields.end()) {
                    auto extensiblesArray = extensibles.value();
                    int numExtensibles = extensiblesArray.size();

                    // Allocate and initialize object arrays that are sized by the number of extensible field groups
                    state.dataSystemAvailabilityManager->SysAvailMgrListData(Item).NumItems = numExtensibles;
                    state.dataSystemAvailabilityManager->SysAvailMgrListData(Item).AvailManagerName.allocate(numExtensibles);
                    state.dataSystemAvailabilityManager->SysAvailMgrListData(Item).cAvailManagerType.allocate(numExtensibles);
                    state.dataSystemAvailabilityManager->SysAvailMgrListData(Item).AvailManagerType.allocate(numExtensibles);
                    for (int extItem = 1; extItem <= numExtensibles; ++extItem) {
                        state.dataSystemAvailabilityManager->SysAvailMgrListData(Item).AvailManagerName = "";
                        state.dataSystemAvailabilityManager->SysAvailMgrListData(Item).cAvailManagerType = "";
                        state.dataSystemAvailabilityManager->SysAvailMgrListData(Item).AvailManagerType = 0;
                    }

                    // Loop through each extensible field group
                    int listItem = 0;
                    for (auto extensibleInstance : extensiblesArray) {
                        ++listItem;
                        state.dataSystemAvailabilityManager->SysAvailMgrListData(Item).AvailManagerName(listItem) =
                            state.dataInputProcessing->inputProcessor->getAlphaFieldValue(
                                state, cCurrentModuleObject, extensibleInstance, extensionSchemaProps, "availability_manager_name");
                        std::string availManagerObjType = state.dataInputProcessing->inputProcessor->getAlphaFieldValue(
                            state, cCurrentModuleObject, extensibleInstance, extensionSchemaProps, "availability_manager_object_type");
                        state.dataSystemAvailabilityManager->SysAvailMgrListData(Item).cAvailManagerType(listItem) = availManagerObjType;
                        state.dataSystemAvailabilityManager->SysAvailMgrListData(Item).AvailManagerType(listItem) =
                            ValidateAndSetSysAvailabilityManagerType(state, availManagerObjType);
                    }
                }

## Questions ##
1. Do we want more functions to help with some of the setup shown here?
