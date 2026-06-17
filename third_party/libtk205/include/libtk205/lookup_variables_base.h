#ifndef LOOKUP_VARIABLES_BASE_H_
#define LOOKUP_VARIABLES_BASE_H_

#include <memory>
#include <vector>
#include <iostream>
#include <sstream>

class PerformanceMapBase;

// ------------------------------------------------------------------------------------------------
/// @class LookupVariablesBase lookup_variables_base.h

class LookupVariablesBase {

public:
    LookupVariablesBase() = default;
    virtual ~LookupVariablesBase() = default;
    LookupVariablesBase(const LookupVariablesBase& other) = default;
    LookupVariablesBase& operator=(const LookupVariablesBase& other) = default;

    virtual void populate_performance_map(PerformanceMapBase* performance_map) = 0;

    inline void add_data_table(PerformanceMapBase* performance_map, std::vector<double>& table)
    {
       performance_map->add_data_table(table);
    }
};

#endif