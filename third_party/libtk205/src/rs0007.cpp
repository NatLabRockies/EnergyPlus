#include <rs0007.h>
#include <loadobject_205.h>

namespace tk205  {

	namespace rs0007_ns  {
	
		const std::string_view Schema::schema_title = "Mechanical Drive";

		const std::string_view Schema::schema_version = "1.0.0";

		const std::string_view Schema::schema_description = "Schema for ASHRAE 205 annex RS0007: Mechanical Drive";

		void from_json(const nlohmann::json& j, ProductInformation& x) {
			a205_json_get<std::string>(j, *RS0007::logger, "manufacturer", x.manufacturer, x.manufacturer_is_set, false);
			a205_json_get<ashrae205_ns::Pattern>(j, *RS0007::logger, "model_number", x.model_number, x.model_number_is_set, false);
			a205_json_get<rs0007_ns::DriveType>(j, *RS0007::logger, "drive_type", x.drive_type, x.drive_type_is_set, false);
		}
		const std::string_view ProductInformation::manufacturer_units = "";

		const std::string_view ProductInformation::model_number_units = "";

		const std::string_view ProductInformation::drive_type_units = "";

		const std::string_view ProductInformation::manufacturer_description = "Manufacturer name";

		const std::string_view ProductInformation::model_number_description = "Model number";

		const std::string_view ProductInformation::drive_type_description = "Type of mechanical drive";

		const std::string_view ProductInformation::manufacturer_name = "manufacturer";

		const std::string_view ProductInformation::model_number_name = "model_number";

		const std::string_view ProductInformation::drive_type_name = "drive_type";

		void from_json(const nlohmann::json& j, Description& x) {
			a205_json_get<rs0007_ns::ProductInformation>(j, *RS0007::logger, "product_information", x.product_information, x.product_information_is_set, false);
		}
		const std::string_view Description::product_information_units = "";

		const std::string_view Description::product_information_description = "Data group describing product information";

		const std::string_view Description::product_information_name = "product_information";

		void from_json(const nlohmann::json& j, GridVariables& x) {
			a205_json_get<std::vector<double>>(j, *RS0007::logger, "output_power", x.output_power, x.output_power_is_set, true);
		}
		void GridVariables::populate_performance_map(PerformanceMapBase* performance_map) {
			add_grid_axis(performance_map, output_power);
			performance_map->finalize_grid(RS0007::logger);
		}
		const std::string_view GridVariables::output_power_units = "W";

		const std::string_view GridVariables::output_power_description = "Output shaft power";

		const std::string_view GridVariables::output_power_name = "output_power";

		void from_json(const nlohmann::json& j, LookupVariables& x) {
			a205_json_get<std::vector<double>>(j, *RS0007::logger, "efficiency", x.efficiency, x.efficiency_is_set, true);
		}
		void LookupVariables::populate_performance_map(PerformanceMapBase* performance_map) {
			add_data_table(performance_map, efficiency);
		}
		const std::string_view LookupVariables::efficiency_units = "-";

		const std::string_view LookupVariables::efficiency_description = "Efficiency of drive";

		const std::string_view LookupVariables::efficiency_name = "efficiency";

		void from_json(const nlohmann::json& j, PerformanceMap& x) {
			a205_json_get<rs0007_ns::GridVariables>(j, *RS0007::logger, "grid_variables", x.grid_variables, x.grid_variables_is_set, true);
			x.grid_variables.populate_performance_map(&x);
			a205_json_get<rs0007_ns::LookupVariables>(j, *RS0007::logger, "lookup_variables", x.lookup_variables, x.lookup_variables_is_set, true);
			x.lookup_variables.populate_performance_map(&x);
		}
		void PerformanceMap::initialize(const nlohmann::json& j) {
			a205_json_get<rs0007_ns::GridVariables>(j, *RS0007::logger, "grid_variables", grid_variables, grid_variables_is_set, true);
			grid_variables.populate_performance_map(this);
			a205_json_get<rs0007_ns::LookupVariables>(j, *RS0007::logger, "lookup_variables", lookup_variables, lookup_variables_is_set, true);
			lookup_variables.populate_performance_map(this);
		}
		const std::string_view PerformanceMap::grid_variables_units = "";

		const std::string_view PerformanceMap::lookup_variables_units = "";

		const std::string_view PerformanceMap::grid_variables_description = "Data group describing grid variables for drive performance";

		const std::string_view PerformanceMap::lookup_variables_description = "Data group describing lookup variables for drive performance";

		const std::string_view PerformanceMap::grid_variables_name = "grid_variables";

		const std::string_view PerformanceMap::lookup_variables_name = "lookup_variables";

		LookupVariablesStruct PerformanceMap::calculate_performance(double output_power, Btwxt::InterpolationMethod performance_interpolation_method ) {
			std::vector<double> target {output_power};
			auto v = PerformanceMapBase::calculate_performance(target, performance_interpolation_method);
			LookupVariablesStruct s {v[0], };
			return s;
		}
		void from_json(const nlohmann::json& j, Performance& x) {
			a205_json_get<double>(j, *RS0007::logger, "speed_ratio", x.speed_ratio, x.speed_ratio_is_set, true);
			a205_json_get<rs0007_ns::PerformanceMap>(j, *RS0007::logger, "performance_map", x.performance_map, x.performance_map_is_set, true);
		}
		const std::string_view Performance::speed_ratio_units = "-";

		const std::string_view Performance::performance_map_units = "";

		const std::string_view Performance::speed_ratio_description = "Ratio of input shaft speed to output shaft speed";

		const std::string_view Performance::performance_map_description = "Data group describing drive performance when operating";

		const std::string_view Performance::speed_ratio_name = "speed_ratio";

		const std::string_view Performance::performance_map_name = "performance_map";

		void from_json(const nlohmann::json& j, RS0007& x) {
			a205_json_get<ashrae205_ns::Metadata>(j, *RS0007::logger, "metadata", x.metadata, x.metadata_is_set, true);
			a205_json_get<rs0007_ns::Description>(j, *RS0007::logger, "description", x.description, x.description_is_set, false);
			a205_json_get<rs0007_ns::Performance>(j, *RS0007::logger, "performance", x.performance, x.performance_is_set, true);
		}
		void RS0007::initialize(const nlohmann::json& j) {
			a205_json_get<ashrae205_ns::Metadata>(j, *RS0007::logger, "metadata", metadata, metadata_is_set, true);
			a205_json_get<rs0007_ns::Description>(j, *RS0007::logger, "description", description, description_is_set, false);
			a205_json_get<rs0007_ns::Performance>(j, *RS0007::logger, "performance", performance, performance_is_set, true);
		}
		 std::shared_ptr<Courierr::Courierr> RS0007::logger {};

		const std::string_view RS0007::metadata_units = "";

		const std::string_view RS0007::description_units = "";

		const std::string_view RS0007::performance_units = "";

		const std::string_view RS0007::metadata_description = "Metadata data group";

		const std::string_view RS0007::description_description = "Data group describing product and rating information";

		const std::string_view RS0007::performance_description = "Data group containing performance information";

		const std::string_view RS0007::metadata_name = "metadata";

		const std::string_view RS0007::description_name = "description";

		const std::string_view RS0007::performance_name = "performance";

	}
}

