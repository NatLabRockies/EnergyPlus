#include <rs0004.h>
#include <loadobject_205.h>

namespace tk205  {

	namespace rs0004_ns  {
	
		const std::string_view Schema::schema_title = "Air-to-Air Direct Expansion Refrigerant System";

		const std::string_view Schema::schema_version = "1.0.0";

		const std::string_view Schema::schema_description = "Schema for ASHRAE 205 annex RS0004: Air-to-Air Direct Expansion Refrigerant System";

		void from_json(const nlohmann::json& j, ProductInformation& x) {
			a205_json_get<std::string>(j, *RS0004::logger, "outdoor_unit_manufacturer", x.outdoor_unit_manufacturer, x.outdoor_unit_manufacturer_is_set, false);
			a205_json_get<ashrae205_ns::Pattern>(j, *RS0004::logger, "outdoor_unit_model_number", x.outdoor_unit_model_number, x.outdoor_unit_model_number_is_set, false);
			a205_json_get<std::string>(j, *RS0004::logger, "indoor_unit_manufacturer", x.indoor_unit_manufacturer, x.indoor_unit_manufacturer_is_set, false);
			a205_json_get<ashrae205_ns::Pattern>(j, *RS0004::logger, "indoor_unit_model_number", x.indoor_unit_model_number, x.indoor_unit_model_number_is_set, false);
			a205_json_get<std::string>(j, *RS0004::logger, "refrigerant", x.refrigerant, x.refrigerant_is_set, false);
			a205_json_get<ashrae205_ns::CompressorType>(j, *RS0004::logger, "compressor_type", x.compressor_type, x.compressor_type_is_set, false);
		}
		const std::string_view ProductInformation::outdoor_unit_manufacturer_units = "";

		const std::string_view ProductInformation::outdoor_unit_model_number_units = "";

		const std::string_view ProductInformation::indoor_unit_manufacturer_units = "";

		const std::string_view ProductInformation::indoor_unit_model_number_units = "";

		const std::string_view ProductInformation::refrigerant_units = "";

		const std::string_view ProductInformation::compressor_type_units = "";

		const std::string_view ProductInformation::outdoor_unit_manufacturer_description = "Outdoor unit manufacturer name";

		const std::string_view ProductInformation::outdoor_unit_model_number_description = "Outdoor unit model number";

		const std::string_view ProductInformation::indoor_unit_manufacturer_description = "Indoor unit manufacturer name";

		const std::string_view ProductInformation::indoor_unit_model_number_description = "Indoor unit model number";

		const std::string_view ProductInformation::refrigerant_description = "Refrigerant used";

		const std::string_view ProductInformation::compressor_type_description = "Type of compressor";

		const std::string_view ProductInformation::outdoor_unit_manufacturer_name = "outdoor_unit_manufacturer";

		const std::string_view ProductInformation::outdoor_unit_model_number_name = "outdoor_unit_model_number";

		const std::string_view ProductInformation::indoor_unit_manufacturer_name = "indoor_unit_manufacturer";

		const std::string_view ProductInformation::indoor_unit_model_number_name = "indoor_unit_model_number";

		const std::string_view ProductInformation::refrigerant_name = "refrigerant";

		const std::string_view ProductInformation::compressor_type_name = "compressor_type";

		void from_json(const nlohmann::json& j, Description& x) {
			a205_json_get<rs0004_ns::ProductInformation>(j, *RS0004::logger, "product_information", x.product_information, x.product_information_is_set, false);
		}
		const std::string_view Description::product_information_units = "";

		const std::string_view Description::product_information_description = "Data group describing product information";

		const std::string_view Description::product_information_name = "product_information";

		void from_json(const nlohmann::json& j, GridVariablesCooling& x) {
			a205_json_get<std::vector<double>>(j, *RS0004::logger, "outdoor_coil_entering_dry_bulb_temperature", x.outdoor_coil_entering_dry_bulb_temperature, x.outdoor_coil_entering_dry_bulb_temperature_is_set, true);
			a205_json_get<std::vector<double>>(j, *RS0004::logger, "indoor_coil_entering_relative_humidity", x.indoor_coil_entering_relative_humidity, x.indoor_coil_entering_relative_humidity_is_set, true);
			a205_json_get<std::vector<double>>(j, *RS0004::logger, "indoor_coil_entering_dry_bulb_temperature", x.indoor_coil_entering_dry_bulb_temperature, x.indoor_coil_entering_dry_bulb_temperature_is_set, true);
			a205_json_get<std::vector<double>>(j, *RS0004::logger, "indoor_coil_air_mass_flow_rate", x.indoor_coil_air_mass_flow_rate, x.indoor_coil_air_mass_flow_rate_is_set, true);
			a205_json_get<std::vector<int>>(j, *RS0004::logger, "compressor_sequence_number", x.compressor_sequence_number, x.compressor_sequence_number_is_set, true);
			a205_json_get<std::vector<double>>(j, *RS0004::logger, "ambient_absolute_air_pressure", x.ambient_absolute_air_pressure, x.ambient_absolute_air_pressure_is_set, true);
		}
		void GridVariablesCooling::populate_performance_map(PerformanceMapBase* performance_map) {
			add_grid_axis(performance_map, outdoor_coil_entering_dry_bulb_temperature);
			add_grid_axis(performance_map, indoor_coil_entering_relative_humidity);
			add_grid_axis(performance_map, indoor_coil_entering_dry_bulb_temperature);
			add_grid_axis(performance_map, indoor_coil_air_mass_flow_rate);
			add_grid_axis(performance_map, compressor_sequence_number);
			add_grid_axis(performance_map, ambient_absolute_air_pressure);
			performance_map->finalize_grid(RS0004::logger);
		}
		const std::string_view GridVariablesCooling::outdoor_coil_entering_dry_bulb_temperature_units = "K";

		const std::string_view GridVariablesCooling::indoor_coil_entering_relative_humidity_units = "-";

		const std::string_view GridVariablesCooling::indoor_coil_entering_dry_bulb_temperature_units = "K";

		const std::string_view GridVariablesCooling::indoor_coil_air_mass_flow_rate_units = "kg/s";

		const std::string_view GridVariablesCooling::compressor_sequence_number_units = "-";

		const std::string_view GridVariablesCooling::ambient_absolute_air_pressure_units = "Pa";

		const std::string_view GridVariablesCooling::outdoor_coil_entering_dry_bulb_temperature_description = "Dry bulb temperature of the air entering the outdoor coil";

		const std::string_view GridVariablesCooling::indoor_coil_entering_relative_humidity_description = "Relative humidity of the air entering the indoor coil";

		const std::string_view GridVariablesCooling::indoor_coil_entering_dry_bulb_temperature_description = "Dry bulb temperature of the air entering the indoor coil";

		const std::string_view GridVariablesCooling::indoor_coil_air_mass_flow_rate_description = "Mass flow rate of air entering the indoor coil";

		const std::string_view GridVariablesCooling::compressor_sequence_number_description = "Index indicating the relative capacity order of the compressor speed/stage expressed in order from lowest capacity (starting at 1) to highest capacity";

		const std::string_view GridVariablesCooling::ambient_absolute_air_pressure_description = "Ambient absolute air pressure";

		const std::string_view GridVariablesCooling::outdoor_coil_entering_dry_bulb_temperature_name = "outdoor_coil_entering_dry_bulb_temperature";

		const std::string_view GridVariablesCooling::indoor_coil_entering_relative_humidity_name = "indoor_coil_entering_relative_humidity";

		const std::string_view GridVariablesCooling::indoor_coil_entering_dry_bulb_temperature_name = "indoor_coil_entering_dry_bulb_temperature";

		const std::string_view GridVariablesCooling::indoor_coil_air_mass_flow_rate_name = "indoor_coil_air_mass_flow_rate";

		const std::string_view GridVariablesCooling::compressor_sequence_number_name = "compressor_sequence_number";

		const std::string_view GridVariablesCooling::ambient_absolute_air_pressure_name = "ambient_absolute_air_pressure";

		void from_json(const nlohmann::json& j, LookupVariablesCooling& x) {
			a205_json_get<std::vector<double>>(j, *RS0004::logger, "gross_total_capacity", x.gross_total_capacity, x.gross_total_capacity_is_set, true);
			a205_json_get<std::vector<double>>(j, *RS0004::logger, "gross_sensible_capacity", x.gross_sensible_capacity, x.gross_sensible_capacity_is_set, true);
			a205_json_get<std::vector<double>>(j, *RS0004::logger, "gross_power", x.gross_power, x.gross_power_is_set, true);
		}
		void LookupVariablesCooling::populate_performance_map(PerformanceMapBase* performance_map) {
			add_data_table(performance_map, gross_total_capacity);
			add_data_table(performance_map, gross_sensible_capacity);
			add_data_table(performance_map, gross_power);
		}
		const std::string_view LookupVariablesCooling::gross_total_capacity_units = "W";

		const std::string_view LookupVariablesCooling::gross_sensible_capacity_units = "W";

		const std::string_view LookupVariablesCooling::gross_power_units = "W";

		const std::string_view LookupVariablesCooling::gross_total_capacity_description = "Total heat removed by the indoor coil";

		const std::string_view LookupVariablesCooling::gross_sensible_capacity_description = "Sensible heat removed by the indoor coil";

		const std::string_view LookupVariablesCooling::gross_power_description = "Gross power draw (of the outdoor unit)";

		const std::string_view LookupVariablesCooling::gross_total_capacity_name = "gross_total_capacity";

		const std::string_view LookupVariablesCooling::gross_sensible_capacity_name = "gross_sensible_capacity";

		const std::string_view LookupVariablesCooling::gross_power_name = "gross_power";

		void from_json(const nlohmann::json& j, PerformanceMapCooling& x) {
			a205_json_get<rs0004_ns::GridVariablesCooling>(j, *RS0004::logger, "grid_variables", x.grid_variables, x.grid_variables_is_set, true);
			x.grid_variables.populate_performance_map(&x);
			a205_json_get<rs0004_ns::LookupVariablesCooling>(j, *RS0004::logger, "lookup_variables", x.lookup_variables, x.lookup_variables_is_set, true);
			x.lookup_variables.populate_performance_map(&x);
		}
		void PerformanceMapCooling::initialize(const nlohmann::json& j) {
			a205_json_get<rs0004_ns::GridVariablesCooling>(j, *RS0004::logger, "grid_variables", grid_variables, grid_variables_is_set, true);
			grid_variables.populate_performance_map(this);
			a205_json_get<rs0004_ns::LookupVariablesCooling>(j, *RS0004::logger, "lookup_variables", lookup_variables, lookup_variables_is_set, true);
			lookup_variables.populate_performance_map(this);
		}
		const std::string_view PerformanceMapCooling::grid_variables_units = "";

		const std::string_view PerformanceMapCooling::lookup_variables_units = "";

		const std::string_view PerformanceMapCooling::grid_variables_description = "Data group defining the grid variables for cooling performance";

		const std::string_view PerformanceMapCooling::lookup_variables_description = "Data group defining the lookup variables for cooling performance";

		const std::string_view PerformanceMapCooling::grid_variables_name = "grid_variables";

		const std::string_view PerformanceMapCooling::lookup_variables_name = "lookup_variables";

		LookupVariablesCoolingStruct PerformanceMapCooling::calculate_performance(double outdoor_coil_entering_dry_bulb_temperature, double indoor_coil_entering_relative_humidity, double indoor_coil_entering_dry_bulb_temperature, double indoor_coil_air_mass_flow_rate, double compressor_sequence_number, double ambient_absolute_air_pressure, Btwxt::InterpolationMethod performance_interpolation_method ) {
			std::vector<double> target {outdoor_coil_entering_dry_bulb_temperature, indoor_coil_entering_relative_humidity, indoor_coil_entering_dry_bulb_temperature, indoor_coil_air_mass_flow_rate, compressor_sequence_number, ambient_absolute_air_pressure};
			auto v = PerformanceMapBase::calculate_performance(target, performance_interpolation_method);
			LookupVariablesCoolingStruct s {v[0], v[1], v[2], };
			return s;
		}
		void from_json(const nlohmann::json& j, GridVariablesStandby& x) {
			a205_json_get<std::vector<double>>(j, *RS0004::logger, "outdoor_coil_environment_dry_bulb_temperature", x.outdoor_coil_environment_dry_bulb_temperature, x.outdoor_coil_environment_dry_bulb_temperature_is_set, true);
		}
		void GridVariablesStandby::populate_performance_map(PerformanceMapBase* performance_map) {
			add_grid_axis(performance_map, outdoor_coil_environment_dry_bulb_temperature);
			performance_map->finalize_grid(RS0004::logger);
		}
		const std::string_view GridVariablesStandby::outdoor_coil_environment_dry_bulb_temperature_units = "K";

		const std::string_view GridVariablesStandby::outdoor_coil_environment_dry_bulb_temperature_description = "Dry bulb temperature of the air in the environment of the outdoor coil";

		const std::string_view GridVariablesStandby::outdoor_coil_environment_dry_bulb_temperature_name = "outdoor_coil_environment_dry_bulb_temperature";

		void from_json(const nlohmann::json& j, LookupVariablesStandby& x) {
			a205_json_get<std::vector<double>>(j, *RS0004::logger, "gross_power", x.gross_power, x.gross_power_is_set, true);
		}
		void LookupVariablesStandby::populate_performance_map(PerformanceMapBase* performance_map) {
			add_data_table(performance_map, gross_power);
		}
		const std::string_view LookupVariablesStandby::gross_power_units = "W";

		const std::string_view LookupVariablesStandby::gross_power_description = "Gross power draw (of the outdoor unit)";

		const std::string_view LookupVariablesStandby::gross_power_name = "gross_power";

		void from_json(const nlohmann::json& j, PerformanceMapStandby& x) {
			a205_json_get<rs0004_ns::GridVariablesStandby>(j, *RS0004::logger, "grid_variables", x.grid_variables, x.grid_variables_is_set, true);
			x.grid_variables.populate_performance_map(&x);
			a205_json_get<rs0004_ns::LookupVariablesStandby>(j, *RS0004::logger, "lookup_variables", x.lookup_variables, x.lookup_variables_is_set, true);
			x.lookup_variables.populate_performance_map(&x);
		}
		void PerformanceMapStandby::initialize(const nlohmann::json& j) {
			a205_json_get<rs0004_ns::GridVariablesStandby>(j, *RS0004::logger, "grid_variables", grid_variables, grid_variables_is_set, true);
			grid_variables.populate_performance_map(this);
			a205_json_get<rs0004_ns::LookupVariablesStandby>(j, *RS0004::logger, "lookup_variables", lookup_variables, lookup_variables_is_set, true);
			lookup_variables.populate_performance_map(this);
		}
		const std::string_view PerformanceMapStandby::grid_variables_units = "";

		const std::string_view PerformanceMapStandby::lookup_variables_units = "";

		const std::string_view PerformanceMapStandby::grid_variables_description = "Data group defining the grid variables for standby performance";

		const std::string_view PerformanceMapStandby::lookup_variables_description = "Data group defining the lookup variables for standby performance";

		const std::string_view PerformanceMapStandby::grid_variables_name = "grid_variables";

		const std::string_view PerformanceMapStandby::lookup_variables_name = "lookup_variables";

		LookupVariablesStandbyStruct PerformanceMapStandby::calculate_performance(double outdoor_coil_environment_dry_bulb_temperature, Btwxt::InterpolationMethod performance_interpolation_method ) {
			std::vector<double> target {outdoor_coil_environment_dry_bulb_temperature};
			auto v = PerformanceMapBase::calculate_performance(target, performance_interpolation_method);
			LookupVariablesStandbyStruct s {v[0], };
			return s;
		}
		void from_json(const nlohmann::json& j, Performance& x) {
			a205_json_get<ashrae205_ns::SpeedControlType>(j, *RS0004::logger, "compressor_speed_control_type", x.compressor_speed_control_type, x.compressor_speed_control_type_is_set, true);
			a205_json_get<double>(j, *RS0004::logger, "cycling_degradation_coefficient", x.cycling_degradation_coefficient, x.cycling_degradation_coefficient_is_set, true);
			a205_json_get<rs0004_ns::PerformanceMapCooling>(j, *RS0004::logger, "performance_map_cooling", x.performance_map_cooling, x.performance_map_cooling_is_set, true);
			a205_json_get<rs0004_ns::PerformanceMapStandby>(j, *RS0004::logger, "performance_map_standby", x.performance_map_standby, x.performance_map_standby_is_set, true);
		}
		const std::string_view Performance::compressor_speed_control_type_units = "";

		const std::string_view Performance::cycling_degradation_coefficient_units = "-";

		const std::string_view Performance::performance_map_cooling_units = "";

		const std::string_view Performance::performance_map_standby_units = "";

		const std::string_view Performance::compressor_speed_control_type_description = "Method used to control different speeds of the compressor";

		const std::string_view Performance::cycling_degradation_coefficient_description = "Cycling degradation coefficient (C~D~) as described in AHRI 210/240";

		const std::string_view Performance::performance_map_cooling_description = "Data group describing cooling performance over a range of conditions";

		const std::string_view Performance::performance_map_standby_description = "Data group describing standby performance";

		const std::string_view Performance::compressor_speed_control_type_name = "compressor_speed_control_type";

		const std::string_view Performance::cycling_degradation_coefficient_name = "cycling_degradation_coefficient";

		const std::string_view Performance::performance_map_cooling_name = "performance_map_cooling";

		const std::string_view Performance::performance_map_standby_name = "performance_map_standby";

		void from_json(const nlohmann::json& j, RS0004& x) {
			a205_json_get<ashrae205_ns::Metadata>(j, *RS0004::logger, "metadata", x.metadata, x.metadata_is_set, true);
			a205_json_get<rs0004_ns::Description>(j, *RS0004::logger, "description", x.description, x.description_is_set, false);
			a205_json_get<rs0004_ns::Performance>(j, *RS0004::logger, "performance", x.performance, x.performance_is_set, true);
		}
		void RS0004::initialize(const nlohmann::json& j) {
			a205_json_get<ashrae205_ns::Metadata>(j, *RS0004::logger, "metadata", metadata, metadata_is_set, true);
			a205_json_get<rs0004_ns::Description>(j, *RS0004::logger, "description", description, description_is_set, false);
			a205_json_get<rs0004_ns::Performance>(j, *RS0004::logger, "performance", performance, performance_is_set, true);
		}
		 std::shared_ptr<Courierr::Courierr> RS0004::logger {};

		const std::string_view RS0004::metadata_units = "";

		const std::string_view RS0004::description_units = "";

		const std::string_view RS0004::performance_units = "";

		const std::string_view RS0004::metadata_description = "Metadata data group";

		const std::string_view RS0004::description_description = "Data group describing product and rating information";

		const std::string_view RS0004::performance_description = "Data group containing performance information";

		const std::string_view RS0004::metadata_name = "metadata";

		const std::string_view RS0004::description_name = "description";

		const std::string_view RS0004::performance_name = "performance";

	}
}

