#pragma once

#include <valijson/schema.hpp>
#include <valijson/validation_visitor.hpp>

namespace valijson {

class Schema;
class ValidationResults;


/**
 * @brief   Class that provides validation functionality.
 *
 * @tparam  RegexEngine regular expression engine used for pattern constraint validation.

 */
template <typename RegexEngine>
class ValidatorT
{
public:
    enum TypeCheckingMode
    {
        kStrongTypes,
        kWeakTypes
    };

    enum DateTimeMode
    {
        kStrictDateTime,
        kPermissiveDateTime
    };

    /**
     * @brief  Construct a Validator that uses strong type checking by default
     */
    ValidatorT()
      : strictTypes(true)
      , strictDateTime(true)
    { }

    /**
     * @brief  Construct a Validator using a specific type checking mode
     *
     * @param  typeCheckingMode  choice of strong or weak type checking
     */
    ValidatorT(TypeCheckingMode typeCheckingMode, DateTimeMode dateTimeMode = kStrictDateTime)
      : strictTypes(typeCheckingMode == kStrongTypes)
      , strictDateTime(dateTimeMode == kStrictDateTime)
    { }

    /**
     * @brief  Validate a JSON document and optionally return the results.
     *
     * When a ValidationResults object is provided via the \c results parameter,
     * validation will be performed against each constraint defined by the
     * schema, even if validation fails for some or all constraints.
     *
     * If a pointer to a ValidationResults instance is not provided, validation
     * will only continue for as long as the constraints are validated
     * successfully.
     *
     * @param  schema   The schema to validate against
     * @param  target   A rapidjson::Value to be validated
     *
     * @param  results  An optional pointer to a ValidationResults instance that
     *                  will be used to report validation errors
     *
     * @returns  true if validation succeeds, false otherwise
     */
    template<typename AdapterType>
    bool validate(const Subschema &schema, const AdapterType &target,
            ValidationResults *results)
    {
        // Construct a ValidationVisitor to perform validation at the root level
        ValidationVisitor<AdapterType, RegexEngine> v(
                target,
                std::vector<std::string>(1, "<root>"),
                strictTypes,
                strictDateTime,
                results,
                regexesCache);

        return v.validateSchema(schema);
    }

private:

    /// Flag indicating that strict type comparisons should be used
    bool strictTypes;

    /// Parse date/time values strictly, according to RFC-3999
    bool strictDateTime;

    /// Cached regex objects for pattern constraint. Key - pattern.
    std::unordered_map<std::string, RegexEngine> regexesCache;
};

/**
 * @brief   Struct that provides a default Regular Expression Engine using std::regex
 */
struct DefaultRegexEngine
{
    DefaultRegexEngine(const std::string& pattern)
      : regex(pattern) { }

    static bool search(const std::string& s, const DefaultRegexEngine& r)
    {
        return std::regex_search(s, r.regex);
    }

private:
    std::regex regex;
};

using Validator = ValidatorT<DefaultRegexEngine>;

}  // namespace valijson
