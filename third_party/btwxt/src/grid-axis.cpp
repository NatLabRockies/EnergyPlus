/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

// btwxt
#include <btwxt/btwxt.h>

namespace Btwxt {

GridAxis::GridAxis(std::vector<double> values_in,
                   const std::string& name,
                   InterpolationMethod interpolation_method,
                   ExtrapolationMethod extrapolation_method,
                   std::pair<double, double> extrapolation_limits,
                   const std::shared_ptr<Courierr::Courierr>& logger_in)
    : name(name)
    , values(std::move(values_in))
    , interpolation_method(interpolation_method)
    , extrapolation_method(extrapolation_method)
    , extrapolation_limits(std::move(extrapolation_limits))
    , cubic_spacing_ratios(
          2, std::vector<double>(std::max(static_cast<int>(values.size()) - 1, 0), 1.0))
    , logger(logger_in)
{
    if (values.empty()) {
        throw BtwxtException(
            std::format("Cannot create grid axis (name=\"{}\") from a zero-length vector.", name),
            *logger);
    }
    check_grid_sorted();
    check_extrapolation_limits();
    if (interpolation_method == InterpolationMethod::cubic) {
        calculate_cubic_spacing_ratios();
    }
}

void GridAxis::set_interpolation_method(InterpolationMethod interpolation_method_in)
{
    interpolation_method = interpolation_method_in;
    if (interpolation_method_in == InterpolationMethod::cubic) {
        calculate_cubic_spacing_ratios();
    }
}

void GridAxis::set_extrapolation_method(ExtrapolationMethod extrapolation_method_in)
{
    constexpr std::string_view info_format =
        "A {} extrapolation method is not valid for grid axis (name=\"{}\") with only {} value. "
        "Extrapolation method reset to {}.";
    switch (extrapolation_method_in) {
    case ExtrapolationMethod::linear: {
        if (get_length() == 1) {
            extrapolation_method = ExtrapolationMethod::constant;
            logger->info(std::format(info_format, "linear", name, "one", "constant"));
            return;
        }
        break;
    }
    default: {
        break;
    }
    }
    extrapolation_method = extrapolation_method_in;
}

void GridAxis::calculate_cubic_spacing_ratios()
{
    if (get_length() == 1) {
        interpolation_method = InterpolationMethod::linear;
        logger->info(std::format(
            "A cubic interpolation method is not valid for grid axis (name=\"{}\") with "
            "only one value. Interpolation method reset to linear.",
            name));
    }
    if (interpolation_method == InterpolationMethod::linear) {
        return;
    }
    static constexpr std::size_t floor = 0;
    static constexpr std::size_t ceiling = 1;
    for (std::size_t i = 0; i < values.size() - 1; i++) {
        double center_spacing = values[i + 1] - values[i];
        if (i != 0) {
            cubic_spacing_ratios[floor][i] = center_spacing / (values[i + 1] - values[i - 1]);
        }
        if (i + 2 != values.size()) {
            cubic_spacing_ratios[ceiling][i] = center_spacing / (values[i + 2] - values[i]);
        }
    }
}

const std::vector<double>&
GridAxis::get_cubic_spacing_ratios(const std::size_t floor_or_ceiling) const
{
    return cubic_spacing_ratios[floor_or_ceiling];
}

void GridAxis::check_grid_sorted()
{
    bool grid_is_sorted = vector_is_valid(values);
    if (!grid_is_sorted) {
        throw BtwxtException(std::format("Grid axis (name=\"{}\") values are not sorted, or have duplicates.", name),
                             *logger);
    }
}

void GridAxis::check_extrapolation_limits()
{
    constexpr std::string_view info_format {
        "Grid axis (name=\"{}\") {} extrapolation limit ({}) is within the set of grid axis "
        "values. Setting to {} axis value ({})."};
    if (extrapolation_limits.first > values[0]) {
        logger->info(std::format(
            info_format, name, "lower", extrapolation_limits.first, "smallest", values[0]));
        extrapolation_limits.first = values[0];
    }
    if (extrapolation_limits.second < values.back()) {
        logger->info(std::format(
            info_format, name, "upper", extrapolation_limits.second, "largest", values.back()));
        extrapolation_limits.second = values.back();
    }
}

// return an evenly spaced 1-d vector of doubles.
std::vector<double> linspace(double start, double stop, std::size_t number_of_points)
{
    std::vector<double> result(number_of_points);
    double step = (stop - start) / (static_cast<double>(number_of_points) - 1.);
    double value = start;
    for (std::size_t i = 0; i < number_of_points; i++) {
        result[i] = value;
        value += step;
    }
    return result;
}

} // namespace Btwxt
