#include <rs0005.h>
#include <loadobject_205.h>

namespace tk205  {

	namespace rs0005_ns  {
	
		const std::string_view Schema::schema_title = "Motor";

		const std::string_view Schema::schema_version = "1.0.0";

		const std::string_view Schema::schema_description = "Schema for ASHRAE 205 annex RS0005: Motor";

		void from_json(const nlohmann::json& j, ProductInformation& x) {
			a205_json_get<std::string>(j, *RS0005::logger, "manufacturer", x.manufacturer, x.manufacturer_is_set, false);
			a205_json_get<ashrae205_ns::Pattern>(j, *RS0005::logger, "model_number", x.model_number, x.model_number_is_set, false);
			a205_json_get<double>(j, *RS0005::logger, "nominal_voltage", x.nominal_voltage, x.nominal_voltage_is_set, false);
			a205_json_get<double>(j, *RS0005::logger, "nominal_frequency", x.nominal_frequency, x.nominal_frequency_is_set, false);
		}
		const std::string_view ProductInformation::manufacturer_units = "";

		const std::string_view ProductInformation::model_number_units = "";

		const std::string_view ProductInformation::nominal_voltage_units = "V";

		const std::string_view ProductInformation::nominal_frequency_units = "Hz";

		const std::string_view ProductInformation::manufacturer_description = "Manufacturer name";

		const std::string_view ProductInformation::model_number_description = "Model number";

		const std::string_view ProductInformation::nominal_voltage_description = "Nominal voltage";

		const std::string_view ProductInformation::nominal_frequency_description = "Nominal frequency";

		const std::string_view ProductInformation::manufacturer_name = "manufacturer";

		const std::string_view ProductInformation::model_number_name = "model_number";

		const std::string_view ProductInformation::nominal_voltage_name = "nominal_voltage";

		const std::string_view ProductInformation::nominal_frequency_name = "nominal_frequency";

		void from_json(const nlohmann::json& j, Description& x) {
			a205_json_get<rs0005_ns::ProductInformation>(j, *RS0005::logger, "product_information", x.product_information, x.product_information_is_set, false);
		}
		const std::string_view Description::product_information_units = "";

		const std::string_view Description::product_information_description = "Data group describing product information";

		const std::string_view Description::product_information_name = "product_information";

		void from_json(const nlohmann::json& j, GridVariables& x) {
			a205_json_get<std::vector<double>>(j, *RS0005::logger, "shaft_power", x.shaft_power, x.shaft_power_is_set, true);
			a205_json_get<std::vector<double>>(j, *RS0005::logger, "shaft_rotational_speed", x.shaft_rotational_speed, x.shaft_rotational_speed_is_set, true);
		}
		void GridVariables::populate_performance_map(PerformanceMapBase* performance_map) {
			add_grid_axis(performance_map, shaft_power);
			add_grid_axis(performance_map, shaft_rotational_speed);
			performance_map->finalize_grid(RS0005::logger);
		}
		const std::string_view GridVariables::shaft_power_units = "W";

		const std::string_view GridVariables::shaft_rotational_speed_units = "rev/s";

		const std::string_view GridVariables::shaft_power_description = "Delivered rotational shaft power";

		const std::string_view GridVariables::shaft_rotational_speed_description = "Rotational speed of shaft";

		const std::string_view GridVariables::shaft_power_name = "shaft_power";

		const std::string_view GridVariables::shaft_rotational_speed_name = "shaft_rotational_speed";

		void from_json(const nlohmann::json& j, LookupVariables& x) {
			a205_json_get<std::vector<double>>(j, *RS0005::logger, "efficiency", x.efficiency, x.efficiency_is_set, true);
			a205_json_get<std::vector<double>>(j, *RS0005::logger, "power_factor", x.power_factor, x.power_factor_is_set, true);
		}
		void LookupVariables::populate_performance_map(PerformanceMapBase* performance_map) {
			add_data_table(performance_map, efficiency);
			add_data_table(performance_map, power_factor);
		}
		const std::string_view LookupVariables::efficiency_units = "-";

		const std::string_view LookupVariables::power_factor_units = "-";

		const std::string_view LookupVariables::efficiency_description = "Efficiency of motor";

		const std::string_view LookupVariables::power_factor_description = "Power factor of the motor";

		const std::string_view LookupVariables::efficiency_name = "efficiency";

		const std::string_view LookupVariables::power_factor_name = "power_factor";

		void from_json(const nlohmann::json& j, PerformanceMap& x) {
			a205_json_get<rs0005_ns::GridVariables>(j, *RS0005::logger, "grid_variables", x.grid_variables, x.grid_variables_is_set, true);
			x.grid_variables.populate_performance_map(&x);
			a205_json_get<rs0005_ns::LookupVariables>(j, *RS0005::logger, "lookup_variables", x.lookup_variables, x.lookup_variables_is_set, true);
			x.lookup_variables.populate_performance_map(&x);
		}
		void PerformanceMap::initialize(const nlohmann::json& j) {
			a205_json_get<rs0005_ns::GridVariables>(j, *RS0005::logger, "grid_variables", grid_variables, grid_variables_is_set, true);
			grid_variables.populate_performance_map(this);
			a205_json_get<rs0005_ns::LookupVariables>(j, *RS0005::logger, "lookup_variables", lookup_variables, lookup_variables_is_set, true);
			lookup_variables.populate_performance_map(this);
		}
		const std::string_view PerformanceMap::grid_variables_units = "";

		const std::string_view PerformanceMap::lookup_variables_units = "";

		const std::string_view PerformanceMap::grid_variables_description = "Data group describing grid variables for motor performance";

		const std::string_view PerformanceMap::lookup_variables_description = "Data group describing lookup variables for motor performance";

		const std::string_view PerformanceMap::grid_variables_name = "grid_variables";

		const std::string_view PerformanceMap::lookup_variables_name = "lookup_variables";

		LookupVariablesStruct PerformanceMap::calculate_performance(double shaft_power, double shaft_rotational_speed, Btwxt::InterpolationMethod performance_interpolation_method ) {
			std::vector<double> target {shaft_power, shaft_rotational_speed};
			auto v = PerformanceMapBase::calculate_performance(target, performance_interpolation_method);
			LookupVariablesStruct s {v[0], v[1], };
			return s;
		}
		void from_json(const nlohmann::json& j, Performance& x) {
			a205_json_get<double>(j, *RS0005::logger, "maximum_power", x.maximum_power, x.maximum_power_is_set, true);
			a205_json_get<double>(j, *RS0005::logger, "standby_power", x.standby_power, x.standby_power_is_set, true);
			a205_json_get<int>(j, *RS0005::logger, "number_of_poles", x.number_of_poles, x.number_of_poles_is_set, true);
			a205_json_get<rs0006_ns::RS0006>(j, *RS0005::logger, "drive_representation", x.drive_representation, x.drive_representation_is_set, false);
			a205_json_get<rs0005_ns::PerformanceMap>(j, *RS0005::logger, "performance_map", x.performance_map, x.performance_map_is_set, false);
		}
		const std::string_view Performance::maximum_power_units = "W";

		const std::string_view Performance::standby_power_units = "W";

		const std::string_view Performance::number_of_poles_units = "";

		const std::string_view Performance::drive_representation_units = "";

		const std::string_view Performance::performance_map_units = "";

		const std::string_view Performance::maximum_power_description = "Maximum operational input power to the motor";

		const std::string_view Performance::standby_power_description = "Power draw when motor is not operating";

		const std::string_view Performance::number_of_poles_description = "Number of poles";

		const std::string_view Performance::drive_representation_description = "The corresponding Standard 205 drive representation";

		const std::string_view Performance::performance_map_description = "Data group describing motor performance when operating";

		const std::string_view Performance::maximum_power_name = "maximum_power";

		const std::string_view Performance::standby_power_name = "standby_power";

		const std::string_view Performance::number_of_poles_name = "number_of_poles";

		const std::string_view Performance::drive_representation_name = "drive_representation";

		const std::string_view Performance::performance_map_name = "performance_map";

		void from_json(const nlohmann::json& j, RS0005& x) {
			a205_json_get<ashrae205_ns::Metadata>(j, *RS0005::logger, "metadata", x.metadata, x.metadata_is_set, true);
			a205_json_get<rs0005_ns::Description>(j, *RS0005::logger, "description", x.description, x.description_is_set, false);
			a205_json_get<rs0005_ns::Performance>(j, *RS0005::logger, "performance", x.performance, x.performance_is_set, true);
		}
		void RS0005::initialize(const nlohmann::json& j) {
			a205_json_get<ashrae205_ns::Metadata>(j, *RS0005::logger, "metadata", metadata, metadata_is_set, true);
			a205_json_get<rs0005_ns::Description>(j, *RS0005::logger, "description", description, description_is_set, false);
			a205_json_get<rs0005_ns::Performance>(j, *RS0005::logger, "performance", performance, performance_is_set, true);
		}
		 std::shared_ptr<Courierr::Courierr> RS0005::logger {};

		const std::string_view RS0005::metadata_units = "";

		const std::string_view RS0005::description_units = "";

		const std::string_view RS0005::performance_units = "";

		const std::string_view RS0005::metadata_description = "Metadata data group";

		const std::string_view RS0005::description_description = "Data group describing product and rating information";

		const std::string_view RS0005::performance_description = "Data group containing performance information";

		const std::string_view RS0005::metadata_name = "metadata";

		const std::string_view RS0005::description_name = "description";

		const std::string_view RS0005::performance_name = "performance";

	}
}

