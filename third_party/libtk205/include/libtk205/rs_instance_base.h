#ifndef RS_INSTANCE_BASE_H_
#define RS_INSTANCE_BASE_H_

#include <fstream>
#include <memory>
#include <nlohmann/json.hpp>

/// @class RSInstanceBase RS_instance_base.h
/// @brief This class is an abstract base for representation specification classes. It handles no resources.

namespace tk205  {

    class RSInstanceBase
    {
    public:

        RSInstanceBase() = default;
        virtual ~RSInstanceBase() = default;
        RSInstanceBase(const RSInstanceBase& other) = default;
        RSInstanceBase& operator=(const RSInstanceBase& other) = default;
        RSInstanceBase(RSInstanceBase&&) = default;
        RSInstanceBase& operator=(RSInstanceBase&&) = default;

        virtual void initialize(const nlohmann::json& j) = 0;
    };
}

#endif