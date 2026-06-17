#pragma once

#include <map>
#include <string>

#include <valijson/subschema.hpp>

namespace valijson {

typedef std::map<std::string, const Subschema *> SchemaCache;

}  // namespace valijson
