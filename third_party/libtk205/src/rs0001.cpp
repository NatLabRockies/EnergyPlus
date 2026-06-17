#include <rs0001.h>
#include <loadobject_205.h>

namespace tk205  {

	namespace rs0001_ns  {
	
		const std::string_view Schema::schema_title = "Liquid-Cooled Chiller";

		const std::string_view Schema::schema_version = "1.0.0";

		const std::string_view Schema::schema_description = "Schema for ASHRAE 205 annex RS0001: Liquid-Cooled Chiller";

		void from_json(const nlohmann::json& j, ProductInformation& x) {
			a205_json_get<std::string>(j, *RS0001::logger, "manufacturer", x.manufacturer, x.manufacturer_is_set, false);
			a205_json_get<ashrae205_ns::Pattern>(j, *RS0001::logger, "model_number", x.model_number, x.model_number_is_set, false);
			a205_json_get<double>(j, *RS0001::logger, "nominal_voltage", x.nominal_voltage, x.nominal_voltage_is_set, false);
			a205_json_get<double>(j, *RS0001::logger, "nominal_frequency", x.nominal_frequency, x.nominal_frequency_is_set, false);
			a205_json_get<ashrae205_ns::CompressorType>(j, *RS0001::logger, "compressor_type", x.compressor_type, x.compressor_type_is_set, false);
			a205_json_get<std::string>(j, *RS0001::logger, "liquid_data_source", x.liquid_data_source, x.liquid_data_source_is_set, false);
			a205_json_get<std::string>(j, *RS0001::logger, "refrigerant", x.refrigerant, x.refrigerant_is_set, false);
			a205_json_get<bool>(j, *RS0001::logger, "hot_gas_bypass_installed", x.hot_gas_bypass_installed, x.hot_gas_bypass_installed_is_set, false);
		}
		const std::string_view ProductInformation::manufacturer_units = "";

		const std::string_view ProductInformation::model_number_units = "";

		const std::string_view ProductInformation::nominal_voltage_units = "V";

		const std::string_view ProductInformation::nominal_frequency_units = "Hz";

		const std::string_view ProductInformation::compressor_type_units = "";

		const std::string_view ProductInformation::liquid_data_source_units = "";

		const std::string_view ProductInformation::refrigerant_units = "";

		const std::string_view ProductInformation::hot_gas_bypass_installed_units = "";

		const std::string_view ProductInformation::manufacturer_description = "Manufacturer name";

		const std::string_view ProductInformation::model_number_description = "Model number";

		const std::string_view ProductInformation::nominal_voltage_description = "Unit nominal voltage";

		const std::string_view ProductInformation::nominal_frequency_description = "Unit nominal frequency";

		const std::string_view ProductInformation::compressor_type_description = "Type of compressor";

		const std::string_view ProductInformation::liquid_data_source_description = "Source of the liquid properties data";

		const std::string_view ProductInformation::refrigerant_description = "Refrigerant used in the chiller";

		const std::string_view ProductInformation::hot_gas_bypass_installed_description = "Indicates if a hot-gas bypass valve is installed on the chiller";

		const std::string_view ProductInformation::manufacturer_name = "manufacturer";

		const std::string_view ProductInformation::model_number_name = "model_number";

		const std::string_view ProductInformation::nominal_voltage_name = "nominal_voltage";

		const std::string_view ProductInformation::nominal_frequency_name = "nominal_frequency";

		const std::string_view ProductInformation::compressor_type_name = "compressor_type";

		const std::string_view ProductInformation::liquid_data_source_name = "liquid_data_source";

		const std::string_view ProductInformation::refrigerant_name = "refrigerant";

		const std::string_view ProductInformation::hot_gas_bypass_installed_name = "hot_gas_bypass_installed";

		void from_json(const nlohmann::json& j, RatingAHRI550590PartLoadPoint& x) {
			a205_json_get<double>(j, *RS0001::logger, "percent_full_load_capacity", x.percent_full_load_capacity, x.percent_full_load_capacity_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "cooling_capacity", x.cooling_capacity, x.cooling_capacity_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "input_power", x.input_power, x.input_power_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "evaporator_liquid_volumetric_flow_rate", x.evaporator_liquid_volumetric_flow_rate, x.evaporator_liquid_volumetric_flow_rate_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "evaporator_liquid_entering_temperature", x.evaporator_liquid_entering_temperature, x.evaporator_liquid_entering_temperature_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "evaporator_liquid_leaving_temperature", x.evaporator_liquid_leaving_temperature, x.evaporator_liquid_leaving_temperature_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "evaporator_liquid_differential_pressure", x.evaporator_liquid_differential_pressure, x.evaporator_liquid_differential_pressure_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "evaporator_fouling_factor", x.evaporator_fouling_factor, x.evaporator_fouling_factor_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "condenser_liquid_volumetric_flow_rate", x.condenser_liquid_volumetric_flow_rate, x.condenser_liquid_volumetric_flow_rate_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "condenser_liquid_entering_temperature", x.condenser_liquid_entering_temperature, x.condenser_liquid_entering_temperature_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "condenser_liquid_leaving_temperature", x.condenser_liquid_leaving_temperature, x.condenser_liquid_leaving_temperature_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "condenser_liquid_differential_pressure", x.condenser_liquid_differential_pressure, x.condenser_liquid_differential_pressure_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "condenser_fouling_factor", x.condenser_fouling_factor, x.condenser_fouling_factor_is_set, true);
		}
		const std::string_view RatingAHRI550590PartLoadPoint::percent_full_load_capacity_units = "%";

		const std::string_view RatingAHRI550590PartLoadPoint::cooling_capacity_units = "Btu/h";

		const std::string_view RatingAHRI550590PartLoadPoint::input_power_units = "kW";

		const std::string_view RatingAHRI550590PartLoadPoint::evaporator_liquid_volumetric_flow_rate_units = "gpm";

		const std::string_view RatingAHRI550590PartLoadPoint::evaporator_liquid_entering_temperature_units = "F";

		const std::string_view RatingAHRI550590PartLoadPoint::evaporator_liquid_leaving_temperature_units = "F";

		const std::string_view RatingAHRI550590PartLoadPoint::evaporator_liquid_differential_pressure_units = "ft of water";

		const std::string_view RatingAHRI550590PartLoadPoint::evaporator_fouling_factor_units = "h-ft2-F/Btu";

		const std::string_view RatingAHRI550590PartLoadPoint::condenser_liquid_volumetric_flow_rate_units = "gpm";

		const std::string_view RatingAHRI550590PartLoadPoint::condenser_liquid_entering_temperature_units = "F";

		const std::string_view RatingAHRI550590PartLoadPoint::condenser_liquid_leaving_temperature_units = "F";

		const std::string_view RatingAHRI550590PartLoadPoint::condenser_liquid_differential_pressure_units = "ft of water";

		const std::string_view RatingAHRI550590PartLoadPoint::condenser_fouling_factor_units = "h-ft2-F/Btu";

		const std::string_view RatingAHRI550590PartLoadPoint::percent_full_load_capacity_description = "Percent full load cooling capacity";

		const std::string_view RatingAHRI550590PartLoadPoint::cooling_capacity_description = "The actual cooling capacity";

		const std::string_view RatingAHRI550590PartLoadPoint::input_power_description = "Combined power input of all components of the unit, including auxiliary power and excluding integral pumps";

		const std::string_view RatingAHRI550590PartLoadPoint::evaporator_liquid_volumetric_flow_rate_description = "Evaporator liquid volumetric flow rate";

		const std::string_view RatingAHRI550590PartLoadPoint::evaporator_liquid_entering_temperature_description = "Liquid temperature at the entry flange of the evaporator";

		const std::string_view RatingAHRI550590PartLoadPoint::evaporator_liquid_leaving_temperature_description = "Liquid temperature at the exit flange of the evaporator";

		const std::string_view RatingAHRI550590PartLoadPoint::evaporator_liquid_differential_pressure_description = "Pressure difference across the evaporator";

		const std::string_view RatingAHRI550590PartLoadPoint::evaporator_fouling_factor_description = "Factor of heat transfer inhibition due to evaporator heat exchanger fouling layer";

		const std::string_view RatingAHRI550590PartLoadPoint::condenser_liquid_volumetric_flow_rate_description = "Condenser liquid volumetric flow rate";

		const std::string_view RatingAHRI550590PartLoadPoint::condenser_liquid_entering_temperature_description = "Liquid temperature at the entry flange of the condenser";

		const std::string_view RatingAHRI550590PartLoadPoint::condenser_liquid_leaving_temperature_description = "Liquid temperature at the exit flange of the condenser";

		const std::string_view RatingAHRI550590PartLoadPoint::condenser_liquid_differential_pressure_description = "Pressure difference across the condenser";

		const std::string_view RatingAHRI550590PartLoadPoint::condenser_fouling_factor_description = "Factor of heat transfer inhibition due to condenser heat exchanger fouling layer";

		const std::string_view RatingAHRI550590PartLoadPoint::percent_full_load_capacity_name = "percent_full_load_capacity";

		const std::string_view RatingAHRI550590PartLoadPoint::cooling_capacity_name = "cooling_capacity";

		const std::string_view RatingAHRI550590PartLoadPoint::input_power_name = "input_power";

		const std::string_view RatingAHRI550590PartLoadPoint::evaporator_liquid_volumetric_flow_rate_name = "evaporator_liquid_volumetric_flow_rate";

		const std::string_view RatingAHRI550590PartLoadPoint::evaporator_liquid_entering_temperature_name = "evaporator_liquid_entering_temperature";

		const std::string_view RatingAHRI550590PartLoadPoint::evaporator_liquid_leaving_temperature_name = "evaporator_liquid_leaving_temperature";

		const std::string_view RatingAHRI550590PartLoadPoint::evaporator_liquid_differential_pressure_name = "evaporator_liquid_differential_pressure";

		const std::string_view RatingAHRI550590PartLoadPoint::evaporator_fouling_factor_name = "evaporator_fouling_factor";

		const std::string_view RatingAHRI550590PartLoadPoint::condenser_liquid_volumetric_flow_rate_name = "condenser_liquid_volumetric_flow_rate";

		const std::string_view RatingAHRI550590PartLoadPoint::condenser_liquid_entering_temperature_name = "condenser_liquid_entering_temperature";

		const std::string_view RatingAHRI550590PartLoadPoint::condenser_liquid_leaving_temperature_name = "condenser_liquid_leaving_temperature";

		const std::string_view RatingAHRI550590PartLoadPoint::condenser_liquid_differential_pressure_name = "condenser_liquid_differential_pressure";

		const std::string_view RatingAHRI550590PartLoadPoint::condenser_fouling_factor_name = "condenser_fouling_factor";

		void from_json(const nlohmann::json& j, RatingAHRI550590& x) {
			a205_json_get<std::string>(j, *RS0001::logger, "certified_reference_number", x.certified_reference_number, x.certified_reference_number_is_set, true);
			a205_json_get<rs0001_ns::AHRI550590TestStandardYear>(j, *RS0001::logger, "test_standard_year", x.test_standard_year, x.test_standard_year_is_set, true);
			a205_json_get<std::string>(j, *RS0001::logger, "rating_source", x.rating_source, x.rating_source_is_set, false);
			a205_json_get<double>(j, *RS0001::logger, "net_refrigerating_capacity", x.net_refrigerating_capacity, x.net_refrigerating_capacity_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "input_power", x.input_power, x.input_power_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "cop", x.cop, x.cop_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "part_load_value", x.part_load_value, x.part_load_value_is_set, true);
			a205_json_get<std::vector<rs0001_ns::RatingAHRI550590PartLoadPoint>>(j, *RS0001::logger, "part_load_rating_points", x.part_load_rating_points, x.part_load_rating_points_is_set, false);
			a205_json_get<double>(j, *RS0001::logger, "full_load_evaporator_liquid_volumetric_flow_rate", x.full_load_evaporator_liquid_volumetric_flow_rate, x.full_load_evaporator_liquid_volumetric_flow_rate_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "full_load_evaporator_liquid_entering_temperature", x.full_load_evaporator_liquid_entering_temperature, x.full_load_evaporator_liquid_entering_temperature_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "full_load_evaporator_liquid_leaving_temperature", x.full_load_evaporator_liquid_leaving_temperature, x.full_load_evaporator_liquid_leaving_temperature_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "full_load_evaporator_liquid_differential_pressure", x.full_load_evaporator_liquid_differential_pressure, x.full_load_evaporator_liquid_differential_pressure_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "full_load_evaporator_fouling_factor", x.full_load_evaporator_fouling_factor, x.full_load_evaporator_fouling_factor_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "full_load_condenser_liquid_volumetric_flow_rate", x.full_load_condenser_liquid_volumetric_flow_rate, x.full_load_condenser_liquid_volumetric_flow_rate_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "full_load_condenser_liquid_entering_temperature", x.full_load_condenser_liquid_entering_temperature, x.full_load_condenser_liquid_entering_temperature_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "full_load_condenser_liquid_leaving_temperature", x.full_load_condenser_liquid_leaving_temperature, x.full_load_condenser_liquid_leaving_temperature_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "full_load_condenser_liquid_differential_pressure", x.full_load_condenser_liquid_differential_pressure, x.full_load_condenser_liquid_differential_pressure_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "full_load_condenser_fouling_factor", x.full_load_condenser_fouling_factor, x.full_load_condenser_fouling_factor_is_set, true);
			a205_json_get<bool>(j, *RS0001::logger, "rating_recalculatable_from_performance_data", x.rating_recalculatable_from_performance_data, x.rating_recalculatable_from_performance_data_is_set, true);
			a205_json_get<std::string>(j, *RS0001::logger, "rating_recalculatable_explanation", x.rating_recalculatable_explanation, x.rating_recalculatable_explanation_is_set, false);
		}
		const std::string_view RatingAHRI550590::certified_reference_number_units = "";

		const std::string_view RatingAHRI550590::test_standard_year_units = "";

		const std::string_view RatingAHRI550590::rating_source_units = "";

		const std::string_view RatingAHRI550590::net_refrigerating_capacity_units = "Btu/h";

		const std::string_view RatingAHRI550590::input_power_units = "kW";

		const std::string_view RatingAHRI550590::cop_units = "-";

		const std::string_view RatingAHRI550590::part_load_value_units = "-";

		const std::string_view RatingAHRI550590::part_load_rating_points_units = "";

		const std::string_view RatingAHRI550590::full_load_evaporator_liquid_volumetric_flow_rate_units = "gpm";

		const std::string_view RatingAHRI550590::full_load_evaporator_liquid_entering_temperature_units = "F";

		const std::string_view RatingAHRI550590::full_load_evaporator_liquid_leaving_temperature_units = "F";

		const std::string_view RatingAHRI550590::full_load_evaporator_liquid_differential_pressure_units = "ft of water";

		const std::string_view RatingAHRI550590::full_load_evaporator_fouling_factor_units = "h-ft2-F/Btu";

		const std::string_view RatingAHRI550590::full_load_condenser_liquid_volumetric_flow_rate_units = "gpm";

		const std::string_view RatingAHRI550590::full_load_condenser_liquid_entering_temperature_units = "F";

		const std::string_view RatingAHRI550590::full_load_condenser_liquid_leaving_temperature_units = "F";

		const std::string_view RatingAHRI550590::full_load_condenser_liquid_differential_pressure_units = "ft of water";

		const std::string_view RatingAHRI550590::full_load_condenser_fouling_factor_units = "h-ft2-F/Btu";

		const std::string_view RatingAHRI550590::rating_recalculatable_from_performance_data_units = "";

		const std::string_view RatingAHRI550590::rating_recalculatable_explanation_units = "";

		const std::string_view RatingAHRI550590::certified_reference_number_description = "AHRI certified reference number";

		const std::string_view RatingAHRI550590::test_standard_year_description = "Year of the AHRI test standard";

		const std::string_view RatingAHRI550590::rating_source_description = "Source of this rating data";

		const std::string_view RatingAHRI550590::net_refrigerating_capacity_description = "Rated net refrigeration capacity";

		const std::string_view RatingAHRI550590::input_power_description = "Combined power input of all components of the unit, including auxiliary power and excluding integral pumps";

		const std::string_view RatingAHRI550590::cop_description = "Ratio of the net refrigerating capacity to the total input power at the rating conditions";

		const std::string_view RatingAHRI550590::part_load_value_description = "Rated part-load efficiency on the basis of weighted operation at various partial load capacities";

		const std::string_view RatingAHRI550590::part_load_rating_points_description = "The four measured data points used to calculate the part load rating value";

		const std::string_view RatingAHRI550590::full_load_evaporator_liquid_volumetric_flow_rate_description = "Evaporator liquid volumetric flow rate at the full load design point rating condition";

		const std::string_view RatingAHRI550590::full_load_evaporator_liquid_entering_temperature_description = "Liquid temperature at the entry flange of the evaporator at the full load design rating conditions";

		const std::string_view RatingAHRI550590::full_load_evaporator_liquid_leaving_temperature_description = "Liquid temperature at the exit flange of the evaporator at the full load design rating conditions";

		const std::string_view RatingAHRI550590::full_load_evaporator_liquid_differential_pressure_description = "Pressure difference across the evaporator at the full load design rating conditions";

		const std::string_view RatingAHRI550590::full_load_evaporator_fouling_factor_description = "Factor of heat transfer inhibition due to evaporator heat exchanger fouling layer at the full load design rating condition";

		const std::string_view RatingAHRI550590::full_load_condenser_liquid_volumetric_flow_rate_description = "Condenser liquid volumetric flow rate at the full load design rating conditions";

		const std::string_view RatingAHRI550590::full_load_condenser_liquid_entering_temperature_description = "Liquid temperature at the entry flange of the condenser at the full load design rating conditions";

		const std::string_view RatingAHRI550590::full_load_condenser_liquid_leaving_temperature_description = "Liquid temperature at the exit flange of the condenser at the full load design rating conditions";

		const std::string_view RatingAHRI550590::full_load_condenser_liquid_differential_pressure_description = "Pressure difference across the condenser at the full load design rating conditions";

		const std::string_view RatingAHRI550590::full_load_condenser_fouling_factor_description = "Factor of heat transfer inhibition due to condenser heat exchanger fouling layer at the full load design rating conditions";

		const std::string_view RatingAHRI550590::rating_recalculatable_from_performance_data_description = "Whether this rating can be recalculated using the performance data in the representation";

		const std::string_view RatingAHRI550590::rating_recalculatable_explanation_description = "An explanation of the value for `rating_recalculatable_from_performance_data`";

		const std::string_view RatingAHRI550590::certified_reference_number_name = "certified_reference_number";

		const std::string_view RatingAHRI550590::test_standard_year_name = "test_standard_year";

		const std::string_view RatingAHRI550590::rating_source_name = "rating_source";

		const std::string_view RatingAHRI550590::net_refrigerating_capacity_name = "net_refrigerating_capacity";

		const std::string_view RatingAHRI550590::input_power_name = "input_power";

		const std::string_view RatingAHRI550590::cop_name = "cop";

		const std::string_view RatingAHRI550590::part_load_value_name = "part_load_value";

		const std::string_view RatingAHRI550590::part_load_rating_points_name = "part_load_rating_points";

		const std::string_view RatingAHRI550590::full_load_evaporator_liquid_volumetric_flow_rate_name = "full_load_evaporator_liquid_volumetric_flow_rate";

		const std::string_view RatingAHRI550590::full_load_evaporator_liquid_entering_temperature_name = "full_load_evaporator_liquid_entering_temperature";

		const std::string_view RatingAHRI550590::full_load_evaporator_liquid_leaving_temperature_name = "full_load_evaporator_liquid_leaving_temperature";

		const std::string_view RatingAHRI550590::full_load_evaporator_liquid_differential_pressure_name = "full_load_evaporator_liquid_differential_pressure";

		const std::string_view RatingAHRI550590::full_load_evaporator_fouling_factor_name = "full_load_evaporator_fouling_factor";

		const std::string_view RatingAHRI550590::full_load_condenser_liquid_volumetric_flow_rate_name = "full_load_condenser_liquid_volumetric_flow_rate";

		const std::string_view RatingAHRI550590::full_load_condenser_liquid_entering_temperature_name = "full_load_condenser_liquid_entering_temperature";

		const std::string_view RatingAHRI550590::full_load_condenser_liquid_leaving_temperature_name = "full_load_condenser_liquid_leaving_temperature";

		const std::string_view RatingAHRI550590::full_load_condenser_liquid_differential_pressure_name = "full_load_condenser_liquid_differential_pressure";

		const std::string_view RatingAHRI550590::full_load_condenser_fouling_factor_name = "full_load_condenser_fouling_factor";

		const std::string_view RatingAHRI550590::rating_recalculatable_from_performance_data_name = "rating_recalculatable_from_performance_data";

		const std::string_view RatingAHRI550590::rating_recalculatable_explanation_name = "rating_recalculatable_explanation";

		void from_json(const nlohmann::json& j, RatingAHRI551591PartLoadPoint& x) {
			a205_json_get<double>(j, *RS0001::logger, "percent_full_load_capacity", x.percent_full_load_capacity, x.percent_full_load_capacity_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "cooling_capacity", x.cooling_capacity, x.cooling_capacity_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "input_power", x.input_power, x.input_power_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "evaporator_liquid_volumetric_flow_rate", x.evaporator_liquid_volumetric_flow_rate, x.evaporator_liquid_volumetric_flow_rate_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "evaporator_liquid_entering_temperature", x.evaporator_liquid_entering_temperature, x.evaporator_liquid_entering_temperature_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "evaporator_liquid_leaving_temperature", x.evaporator_liquid_leaving_temperature, x.evaporator_liquid_leaving_temperature_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "evaporator_liquid_differential_pressure", x.evaporator_liquid_differential_pressure, x.evaporator_liquid_differential_pressure_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "evaporator_fouling_factor", x.evaporator_fouling_factor, x.evaporator_fouling_factor_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "condenser_liquid_volumetric_flow_rate", x.condenser_liquid_volumetric_flow_rate, x.condenser_liquid_volumetric_flow_rate_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "condenser_liquid_entering_temperature", x.condenser_liquid_entering_temperature, x.condenser_liquid_entering_temperature_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "condenser_liquid_leaving_temperature", x.condenser_liquid_leaving_temperature, x.condenser_liquid_leaving_temperature_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "condenser_liquid_differential_pressure", x.condenser_liquid_differential_pressure, x.condenser_liquid_differential_pressure_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "condenser_fouling_factor", x.condenser_fouling_factor, x.condenser_fouling_factor_is_set, true);
		}
		const std::string_view RatingAHRI551591PartLoadPoint::percent_full_load_capacity_units = "%";

		const std::string_view RatingAHRI551591PartLoadPoint::cooling_capacity_units = "kW";

		const std::string_view RatingAHRI551591PartLoadPoint::input_power_units = "kW";

		const std::string_view RatingAHRI551591PartLoadPoint::evaporator_liquid_volumetric_flow_rate_units = "l/s";

		const std::string_view RatingAHRI551591PartLoadPoint::evaporator_liquid_entering_temperature_units = "C";

		const std::string_view RatingAHRI551591PartLoadPoint::evaporator_liquid_leaving_temperature_units = "C";

		const std::string_view RatingAHRI551591PartLoadPoint::evaporator_liquid_differential_pressure_units = "kPa";

		const std::string_view RatingAHRI551591PartLoadPoint::evaporator_fouling_factor_units = "m2-K/kW";

		const std::string_view RatingAHRI551591PartLoadPoint::condenser_liquid_volumetric_flow_rate_units = "l/s";

		const std::string_view RatingAHRI551591PartLoadPoint::condenser_liquid_entering_temperature_units = "C";

		const std::string_view RatingAHRI551591PartLoadPoint::condenser_liquid_leaving_temperature_units = "C";

		const std::string_view RatingAHRI551591PartLoadPoint::condenser_liquid_differential_pressure_units = "kPa";

		const std::string_view RatingAHRI551591PartLoadPoint::condenser_fouling_factor_units = "m2-K/kW";

		const std::string_view RatingAHRI551591PartLoadPoint::percent_full_load_capacity_description = "Percent full load cooling capacity";

		const std::string_view RatingAHRI551591PartLoadPoint::cooling_capacity_description = "The actual cooling capacity";

		const std::string_view RatingAHRI551591PartLoadPoint::input_power_description = "Combined power input of all components of the unit, including auxiliary power and excluding integral pumps";

		const std::string_view RatingAHRI551591PartLoadPoint::evaporator_liquid_volumetric_flow_rate_description = "Evaporator liquid volumetric flow rate";

		const std::string_view RatingAHRI551591PartLoadPoint::evaporator_liquid_entering_temperature_description = "Liquid temperature at the entry flange of the evaporator";

		const std::string_view RatingAHRI551591PartLoadPoint::evaporator_liquid_leaving_temperature_description = "Liquid temperature at the exit flange of the evaporator";

		const std::string_view RatingAHRI551591PartLoadPoint::evaporator_liquid_differential_pressure_description = "Pressure difference across the evaporator";

		const std::string_view RatingAHRI551591PartLoadPoint::evaporator_fouling_factor_description = "Factor of heat transfer inhibition due to evaporator heat exchanger fouling layer";

		const std::string_view RatingAHRI551591PartLoadPoint::condenser_liquid_volumetric_flow_rate_description = "Condenser liquid volumetric flow rate";

		const std::string_view RatingAHRI551591PartLoadPoint::condenser_liquid_entering_temperature_description = "Liquid temperature at the entry flange of the condenser";

		const std::string_view RatingAHRI551591PartLoadPoint::condenser_liquid_leaving_temperature_description = "Liquid temperature at the exit flange of the condenser";

		const std::string_view RatingAHRI551591PartLoadPoint::condenser_liquid_differential_pressure_description = "Pressure difference across the condenser";

		const std::string_view RatingAHRI551591PartLoadPoint::condenser_fouling_factor_description = "Factor of heat transfer inhibition due to condenser heat exchanger fouling layer";

		const std::string_view RatingAHRI551591PartLoadPoint::percent_full_load_capacity_name = "percent_full_load_capacity";

		const std::string_view RatingAHRI551591PartLoadPoint::cooling_capacity_name = "cooling_capacity";

		const std::string_view RatingAHRI551591PartLoadPoint::input_power_name = "input_power";

		const std::string_view RatingAHRI551591PartLoadPoint::evaporator_liquid_volumetric_flow_rate_name = "evaporator_liquid_volumetric_flow_rate";

		const std::string_view RatingAHRI551591PartLoadPoint::evaporator_liquid_entering_temperature_name = "evaporator_liquid_entering_temperature";

		const std::string_view RatingAHRI551591PartLoadPoint::evaporator_liquid_leaving_temperature_name = "evaporator_liquid_leaving_temperature";

		const std::string_view RatingAHRI551591PartLoadPoint::evaporator_liquid_differential_pressure_name = "evaporator_liquid_differential_pressure";

		const std::string_view RatingAHRI551591PartLoadPoint::evaporator_fouling_factor_name = "evaporator_fouling_factor";

		const std::string_view RatingAHRI551591PartLoadPoint::condenser_liquid_volumetric_flow_rate_name = "condenser_liquid_volumetric_flow_rate";

		const std::string_view RatingAHRI551591PartLoadPoint::condenser_liquid_entering_temperature_name = "condenser_liquid_entering_temperature";

		const std::string_view RatingAHRI551591PartLoadPoint::condenser_liquid_leaving_temperature_name = "condenser_liquid_leaving_temperature";

		const std::string_view RatingAHRI551591PartLoadPoint::condenser_liquid_differential_pressure_name = "condenser_liquid_differential_pressure";

		const std::string_view RatingAHRI551591PartLoadPoint::condenser_fouling_factor_name = "condenser_fouling_factor";

		void from_json(const nlohmann::json& j, RatingAHRI551591& x) {
			a205_json_get<std::string>(j, *RS0001::logger, "certified_reference_number", x.certified_reference_number, x.certified_reference_number_is_set, true);
			a205_json_get<rs0001_ns::AHRI551591TestStandardYear>(j, *RS0001::logger, "test_standard_year", x.test_standard_year, x.test_standard_year_is_set, true);
			a205_json_get<std::string>(j, *RS0001::logger, "rating_source", x.rating_source, x.rating_source_is_set, false);
			a205_json_get<double>(j, *RS0001::logger, "net_refrigerating_capacity", x.net_refrigerating_capacity, x.net_refrigerating_capacity_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "input_power", x.input_power, x.input_power_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "cop", x.cop, x.cop_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "part_load_value", x.part_load_value, x.part_load_value_is_set, true);
			a205_json_get<std::vector<rs0001_ns::RatingAHRI551591PartLoadPoint>>(j, *RS0001::logger, "part_load_rating_points", x.part_load_rating_points, x.part_load_rating_points_is_set, false);
			a205_json_get<double>(j, *RS0001::logger, "full_load_evaporator_liquid_volumetric_flow_rate", x.full_load_evaporator_liquid_volumetric_flow_rate, x.full_load_evaporator_liquid_volumetric_flow_rate_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "full_load_evaporator_liquid_entering_temperature", x.full_load_evaporator_liquid_entering_temperature, x.full_load_evaporator_liquid_entering_temperature_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "full_load_evaporator_liquid_leaving_temperature", x.full_load_evaporator_liquid_leaving_temperature, x.full_load_evaporator_liquid_leaving_temperature_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "full_load_evaporator_liquid_differential_pressure", x.full_load_evaporator_liquid_differential_pressure, x.full_load_evaporator_liquid_differential_pressure_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "full_load_evaporator_fouling_factor", x.full_load_evaporator_fouling_factor, x.full_load_evaporator_fouling_factor_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "full_load_condenser_liquid_volumetric_flow_rate", x.full_load_condenser_liquid_volumetric_flow_rate, x.full_load_condenser_liquid_volumetric_flow_rate_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "full_load_condenser_liquid_entering_temperature", x.full_load_condenser_liquid_entering_temperature, x.full_load_condenser_liquid_entering_temperature_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "full_load_condenser_liquid_leaving_temperature", x.full_load_condenser_liquid_leaving_temperature, x.full_load_condenser_liquid_leaving_temperature_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "full_load_condenser_liquid_differential_pressure", x.full_load_condenser_liquid_differential_pressure, x.full_load_condenser_liquid_differential_pressure_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "full_load_condenser_fouling_factor", x.full_load_condenser_fouling_factor, x.full_load_condenser_fouling_factor_is_set, true);
			a205_json_get<bool>(j, *RS0001::logger, "rating_recalculatable_from_performance_data", x.rating_recalculatable_from_performance_data, x.rating_recalculatable_from_performance_data_is_set, true);
			a205_json_get<std::string>(j, *RS0001::logger, "rating_recalculatable_explanation", x.rating_recalculatable_explanation, x.rating_recalculatable_explanation_is_set, false);
		}
		const std::string_view RatingAHRI551591::certified_reference_number_units = "";

		const std::string_view RatingAHRI551591::test_standard_year_units = "";

		const std::string_view RatingAHRI551591::rating_source_units = "";

		const std::string_view RatingAHRI551591::net_refrigerating_capacity_units = "kW";

		const std::string_view RatingAHRI551591::input_power_units = "kW";

		const std::string_view RatingAHRI551591::cop_units = "-";

		const std::string_view RatingAHRI551591::part_load_value_units = "-";

		const std::string_view RatingAHRI551591::part_load_rating_points_units = "";

		const std::string_view RatingAHRI551591::full_load_evaporator_liquid_volumetric_flow_rate_units = "l/s";

		const std::string_view RatingAHRI551591::full_load_evaporator_liquid_entering_temperature_units = "C";

		const std::string_view RatingAHRI551591::full_load_evaporator_liquid_leaving_temperature_units = "C";

		const std::string_view RatingAHRI551591::full_load_evaporator_liquid_differential_pressure_units = "kPa";

		const std::string_view RatingAHRI551591::full_load_evaporator_fouling_factor_units = "m2-K/kW";

		const std::string_view RatingAHRI551591::full_load_condenser_liquid_volumetric_flow_rate_units = "l/s";

		const std::string_view RatingAHRI551591::full_load_condenser_liquid_entering_temperature_units = "C";

		const std::string_view RatingAHRI551591::full_load_condenser_liquid_leaving_temperature_units = "C";

		const std::string_view RatingAHRI551591::full_load_condenser_liquid_differential_pressure_units = "kPa";

		const std::string_view RatingAHRI551591::full_load_condenser_fouling_factor_units = "m2-K/kW";

		const std::string_view RatingAHRI551591::rating_recalculatable_from_performance_data_units = "";

		const std::string_view RatingAHRI551591::rating_recalculatable_explanation_units = "";

		const std::string_view RatingAHRI551591::certified_reference_number_description = "AHRI certified reference number";

		const std::string_view RatingAHRI551591::test_standard_year_description = "Year of the AHRI test standard";

		const std::string_view RatingAHRI551591::rating_source_description = "Source of this rating data";

		const std::string_view RatingAHRI551591::net_refrigerating_capacity_description = "Rated net refrigeration capacity";

		const std::string_view RatingAHRI551591::input_power_description = "Combined power input of all components of the unit, including auxiliary power and excluding integral pumps";

		const std::string_view RatingAHRI551591::cop_description = "Ratio of the net refrigerating capacity to the total input power at the rating conditions";

		const std::string_view RatingAHRI551591::part_load_value_description = "Rated part-load efficiency on the basis of weighted operation at various partial load capacities";

		const std::string_view RatingAHRI551591::part_load_rating_points_description = "The four measured data points used to calculate the part load rating value";

		const std::string_view RatingAHRI551591::full_load_evaporator_liquid_volumetric_flow_rate_description = "Evaporator liquid volumetric flow rate at the full load design rating conditions";

		const std::string_view RatingAHRI551591::full_load_evaporator_liquid_entering_temperature_description = "Liquid temperature at the entry flange of the evaporator at the full load design rating conditions";

		const std::string_view RatingAHRI551591::full_load_evaporator_liquid_leaving_temperature_description = "Liquid temperature at the exit flange of the evaporator at the full load design rating conditions";

		const std::string_view RatingAHRI551591::full_load_evaporator_liquid_differential_pressure_description = "Pressure difference across the evaporator at the full load design rating conditions";

		const std::string_view RatingAHRI551591::full_load_evaporator_fouling_factor_description = "Factor of heat transfer inhibition due to evaporator heat exchanger fouling layer at the full load design rating conditions";

		const std::string_view RatingAHRI551591::full_load_condenser_liquid_volumetric_flow_rate_description = "Condenser liquid volumetric flow rate at the full load design rating conditions";

		const std::string_view RatingAHRI551591::full_load_condenser_liquid_entering_temperature_description = "Liquid temperature at the entry flange of the condenser at the full load design rating conditions";

		const std::string_view RatingAHRI551591::full_load_condenser_liquid_leaving_temperature_description = "Liquid temperature at the exit flange of the condenser at the full load design rating conditions";

		const std::string_view RatingAHRI551591::full_load_condenser_liquid_differential_pressure_description = "Pressure difference across the condenser at the full load design rating conditions";

		const std::string_view RatingAHRI551591::full_load_condenser_fouling_factor_description = "Factor of heat transfer inhibition due to condenser heat exchanger fouling layer at the full load design rating conditions";

		const std::string_view RatingAHRI551591::rating_recalculatable_from_performance_data_description = "Whether this rating can be recalculated using the performance data in the representation";

		const std::string_view RatingAHRI551591::rating_recalculatable_explanation_description = "An explanation of the value for `rating_recalculatable_from_performance_data`";

		const std::string_view RatingAHRI551591::certified_reference_number_name = "certified_reference_number";

		const std::string_view RatingAHRI551591::test_standard_year_name = "test_standard_year";

		const std::string_view RatingAHRI551591::rating_source_name = "rating_source";

		const std::string_view RatingAHRI551591::net_refrigerating_capacity_name = "net_refrigerating_capacity";

		const std::string_view RatingAHRI551591::input_power_name = "input_power";

		const std::string_view RatingAHRI551591::cop_name = "cop";

		const std::string_view RatingAHRI551591::part_load_value_name = "part_load_value";

		const std::string_view RatingAHRI551591::part_load_rating_points_name = "part_load_rating_points";

		const std::string_view RatingAHRI551591::full_load_evaporator_liquid_volumetric_flow_rate_name = "full_load_evaporator_liquid_volumetric_flow_rate";

		const std::string_view RatingAHRI551591::full_load_evaporator_liquid_entering_temperature_name = "full_load_evaporator_liquid_entering_temperature";

		const std::string_view RatingAHRI551591::full_load_evaporator_liquid_leaving_temperature_name = "full_load_evaporator_liquid_leaving_temperature";

		const std::string_view RatingAHRI551591::full_load_evaporator_liquid_differential_pressure_name = "full_load_evaporator_liquid_differential_pressure";

		const std::string_view RatingAHRI551591::full_load_evaporator_fouling_factor_name = "full_load_evaporator_fouling_factor";

		const std::string_view RatingAHRI551591::full_load_condenser_liquid_volumetric_flow_rate_name = "full_load_condenser_liquid_volumetric_flow_rate";

		const std::string_view RatingAHRI551591::full_load_condenser_liquid_entering_temperature_name = "full_load_condenser_liquid_entering_temperature";

		const std::string_view RatingAHRI551591::full_load_condenser_liquid_leaving_temperature_name = "full_load_condenser_liquid_leaving_temperature";

		const std::string_view RatingAHRI551591::full_load_condenser_liquid_differential_pressure_name = "full_load_condenser_liquid_differential_pressure";

		const std::string_view RatingAHRI551591::full_load_condenser_fouling_factor_name = "full_load_condenser_fouling_factor";

		const std::string_view RatingAHRI551591::rating_recalculatable_from_performance_data_name = "rating_recalculatable_from_performance_data";

		const std::string_view RatingAHRI551591::rating_recalculatable_explanation_name = "rating_recalculatable_explanation";

		void from_json(const nlohmann::json& j, Description& x) {
			a205_json_get<rs0001_ns::ProductInformation>(j, *RS0001::logger, "product_information", x.product_information, x.product_information_is_set, false);
			a205_json_get<rs0001_ns::RatingAHRI550590>(j, *RS0001::logger, "rating_ahri_550_590", x.rating_ahri_550_590, x.rating_ahri_550_590_is_set, false);
			a205_json_get<rs0001_ns::RatingAHRI551591>(j, *RS0001::logger, "rating_ahri_551_591", x.rating_ahri_551_591, x.rating_ahri_551_591_is_set, false);
		}
		const std::string_view Description::product_information_units = "";

		const std::string_view Description::rating_ahri_550_590_units = "";

		const std::string_view Description::rating_ahri_551_591_units = "";

		const std::string_view Description::product_information_description = "Data group describing product information";

		const std::string_view Description::rating_ahri_550_590_description = "Data group containing information relevant to products rated under AHRI 550/590";

		const std::string_view Description::rating_ahri_551_591_description = "Data group containing information relevant to products rated under AHRI 551/591";

		const std::string_view Description::product_information_name = "product_information";

		const std::string_view Description::rating_ahri_550_590_name = "rating_ahri_550_590";

		const std::string_view Description::rating_ahri_551_591_name = "rating_ahri_551_591";

		void from_json(const nlohmann::json& j, GridVariablesCooling& x) {
			a205_json_get<std::vector<double>>(j, *RS0001::logger, "evaporator_liquid_volumetric_flow_rate", x.evaporator_liquid_volumetric_flow_rate, x.evaporator_liquid_volumetric_flow_rate_is_set, true);
			a205_json_get<std::vector<double>>(j, *RS0001::logger, "evaporator_liquid_leaving_temperature", x.evaporator_liquid_leaving_temperature, x.evaporator_liquid_leaving_temperature_is_set, true);
			a205_json_get<std::vector<double>>(j, *RS0001::logger, "condenser_liquid_volumetric_flow_rate", x.condenser_liquid_volumetric_flow_rate, x.condenser_liquid_volumetric_flow_rate_is_set, true);
			a205_json_get<std::vector<double>>(j, *RS0001::logger, "condenser_liquid_entering_temperature", x.condenser_liquid_entering_temperature, x.condenser_liquid_entering_temperature_is_set, true);
			a205_json_get<std::vector<int>>(j, *RS0001::logger, "compressor_sequence_number", x.compressor_sequence_number, x.compressor_sequence_number_is_set, true);
		}
		void GridVariablesCooling::populate_performance_map(PerformanceMapBase* performance_map) {
			add_grid_axis(performance_map, evaporator_liquid_volumetric_flow_rate);
			add_grid_axis(performance_map, evaporator_liquid_leaving_temperature);
			add_grid_axis(performance_map, condenser_liquid_volumetric_flow_rate);
			add_grid_axis(performance_map, condenser_liquid_entering_temperature);
			add_grid_axis(performance_map, compressor_sequence_number);
			performance_map->finalize_grid(RS0001::logger);
		}
		const std::string_view GridVariablesCooling::evaporator_liquid_volumetric_flow_rate_units = "m3/s";

		const std::string_view GridVariablesCooling::evaporator_liquid_leaving_temperature_units = "K";

		const std::string_view GridVariablesCooling::condenser_liquid_volumetric_flow_rate_units = "m3/s";

		const std::string_view GridVariablesCooling::condenser_liquid_entering_temperature_units = "K";

		const std::string_view GridVariablesCooling::compressor_sequence_number_units = "-";

		const std::string_view GridVariablesCooling::evaporator_liquid_volumetric_flow_rate_description = "Chilled liquid (evaporator) flow";

		const std::string_view GridVariablesCooling::evaporator_liquid_leaving_temperature_description = "Leaving evaporator liquid temperature";

		const std::string_view GridVariablesCooling::condenser_liquid_volumetric_flow_rate_description = "Condenser liquid flow";

		const std::string_view GridVariablesCooling::condenser_liquid_entering_temperature_description = "Entering condenser liquid temperature";

		const std::string_view GridVariablesCooling::compressor_sequence_number_description = "Index indicating the relative capacity order of the compressor speed/stage expressed in order from lowest capacity (starting at 1) to highest capacity";

		const std::string_view GridVariablesCooling::evaporator_liquid_volumetric_flow_rate_name = "evaporator_liquid_volumetric_flow_rate";

		const std::string_view GridVariablesCooling::evaporator_liquid_leaving_temperature_name = "evaporator_liquid_leaving_temperature";

		const std::string_view GridVariablesCooling::condenser_liquid_volumetric_flow_rate_name = "condenser_liquid_volumetric_flow_rate";

		const std::string_view GridVariablesCooling::condenser_liquid_entering_temperature_name = "condenser_liquid_entering_temperature";

		const std::string_view GridVariablesCooling::compressor_sequence_number_name = "compressor_sequence_number";

		void from_json(const nlohmann::json& j, LookupVariablesCooling& x) {
			a205_json_get<std::vector<double>>(j, *RS0001::logger, "input_power", x.input_power, x.input_power_is_set, true);
			a205_json_get<std::vector<double>>(j, *RS0001::logger, "net_evaporator_capacity", x.net_evaporator_capacity, x.net_evaporator_capacity_is_set, true);
			a205_json_get<std::vector<double>>(j, *RS0001::logger, "net_condenser_capacity", x.net_condenser_capacity, x.net_condenser_capacity_is_set, true);
			a205_json_get<std::vector<double>>(j, *RS0001::logger, "evaporator_liquid_entering_temperature", x.evaporator_liquid_entering_temperature, x.evaporator_liquid_entering_temperature_is_set, true);
			a205_json_get<std::vector<double>>(j, *RS0001::logger, "condenser_liquid_leaving_temperature", x.condenser_liquid_leaving_temperature, x.condenser_liquid_leaving_temperature_is_set, true);
			a205_json_get<std::vector<double>>(j, *RS0001::logger, "evaporator_liquid_differential_pressure", x.evaporator_liquid_differential_pressure, x.evaporator_liquid_differential_pressure_is_set, true);
			a205_json_get<std::vector<double>>(j, *RS0001::logger, "condenser_liquid_differential_pressure", x.condenser_liquid_differential_pressure, x.condenser_liquid_differential_pressure_is_set, true);
			a205_json_get<std::vector<double>>(j, *RS0001::logger, "oil_cooler_heat", x.oil_cooler_heat, x.oil_cooler_heat_is_set, true);
			a205_json_get<std::vector<double>>(j, *RS0001::logger, "auxiliary_heat", x.auxiliary_heat, x.auxiliary_heat_is_set, true);
		}
		void LookupVariablesCooling::populate_performance_map(PerformanceMapBase* performance_map) {
			add_data_table(performance_map, input_power);
			add_data_table(performance_map, net_evaporator_capacity);
			add_data_table(performance_map, net_condenser_capacity);
			add_data_table(performance_map, evaporator_liquid_entering_temperature);
			add_data_table(performance_map, condenser_liquid_leaving_temperature);
			add_data_table(performance_map, evaporator_liquid_differential_pressure);
			add_data_table(performance_map, condenser_liquid_differential_pressure);
			add_data_table(performance_map, oil_cooler_heat);
			add_data_table(performance_map, auxiliary_heat);
		}
		const std::string_view LookupVariablesCooling::input_power_units = "W";

		const std::string_view LookupVariablesCooling::net_evaporator_capacity_units = "W";

		const std::string_view LookupVariablesCooling::net_condenser_capacity_units = "W";

		const std::string_view LookupVariablesCooling::evaporator_liquid_entering_temperature_units = "K";

		const std::string_view LookupVariablesCooling::condenser_liquid_leaving_temperature_units = "K";

		const std::string_view LookupVariablesCooling::evaporator_liquid_differential_pressure_units = "Pa";

		const std::string_view LookupVariablesCooling::condenser_liquid_differential_pressure_units = "Pa";

		const std::string_view LookupVariablesCooling::oil_cooler_heat_units = "W";

		const std::string_view LookupVariablesCooling::auxiliary_heat_units = "W";

		const std::string_view LookupVariablesCooling::input_power_description = "Total power input";

		const std::string_view LookupVariablesCooling::net_evaporator_capacity_description = "Refrigeration capacity";

		const std::string_view LookupVariablesCooling::net_condenser_capacity_description = "Condenser heat rejection";

		const std::string_view LookupVariablesCooling::evaporator_liquid_entering_temperature_description = "Entering evaporator liquid temperature";

		const std::string_view LookupVariablesCooling::condenser_liquid_leaving_temperature_description = "Leaving condenser liquid temperature";

		const std::string_view LookupVariablesCooling::evaporator_liquid_differential_pressure_description = "Pressure difference across the evaporator";

		const std::string_view LookupVariablesCooling::condenser_liquid_differential_pressure_description = "Pressure difference across the condenser";

		const std::string_view LookupVariablesCooling::oil_cooler_heat_description = "Heat transferred to another liquid crossing the control volume boundary from the chiller oil cooler.";

		const std::string_view LookupVariablesCooling::auxiliary_heat_description = "Heat transferred to another liquid crossing the control volume boundary from the chiller auxiliaries (motor, motor controller, inverter drive, starter, etc).";

		const std::string_view LookupVariablesCooling::input_power_name = "input_power";

		const std::string_view LookupVariablesCooling::net_evaporator_capacity_name = "net_evaporator_capacity";

		const std::string_view LookupVariablesCooling::net_condenser_capacity_name = "net_condenser_capacity";

		const std::string_view LookupVariablesCooling::evaporator_liquid_entering_temperature_name = "evaporator_liquid_entering_temperature";

		const std::string_view LookupVariablesCooling::condenser_liquid_leaving_temperature_name = "condenser_liquid_leaving_temperature";

		const std::string_view LookupVariablesCooling::evaporator_liquid_differential_pressure_name = "evaporator_liquid_differential_pressure";

		const std::string_view LookupVariablesCooling::condenser_liquid_differential_pressure_name = "condenser_liquid_differential_pressure";

		const std::string_view LookupVariablesCooling::oil_cooler_heat_name = "oil_cooler_heat";

		const std::string_view LookupVariablesCooling::auxiliary_heat_name = "auxiliary_heat";

		void from_json(const nlohmann::json& j, PerformanceMapCooling& x) {
			a205_json_get<rs0001_ns::GridVariablesCooling>(j, *RS0001::logger, "grid_variables", x.grid_variables, x.grid_variables_is_set, true);
			x.grid_variables.populate_performance_map(&x);
			a205_json_get<rs0001_ns::LookupVariablesCooling>(j, *RS0001::logger, "lookup_variables", x.lookup_variables, x.lookup_variables_is_set, true);
			x.lookup_variables.populate_performance_map(&x);
		}
		void PerformanceMapCooling::initialize(const nlohmann::json& j) {
			a205_json_get<rs0001_ns::GridVariablesCooling>(j, *RS0001::logger, "grid_variables", grid_variables, grid_variables_is_set, true);
			grid_variables.populate_performance_map(this);
			a205_json_get<rs0001_ns::LookupVariablesCooling>(j, *RS0001::logger, "lookup_variables", lookup_variables, lookup_variables_is_set, true);
			lookup_variables.populate_performance_map(this);
		}
		const std::string_view PerformanceMapCooling::grid_variables_units = "";

		const std::string_view PerformanceMapCooling::lookup_variables_units = "";

		const std::string_view PerformanceMapCooling::grid_variables_description = "Data group defining the grid variables for cooling performance";

		const std::string_view PerformanceMapCooling::lookup_variables_description = "Data group defining the lookup variables for cooling performance";

		const std::string_view PerformanceMapCooling::grid_variables_name = "grid_variables";

		const std::string_view PerformanceMapCooling::lookup_variables_name = "lookup_variables";

		LookupVariablesCoolingStruct PerformanceMapCooling::calculate_performance(double evaporator_liquid_volumetric_flow_rate, double evaporator_liquid_leaving_temperature, double condenser_liquid_volumetric_flow_rate, double condenser_liquid_entering_temperature, double compressor_sequence_number, Btwxt::InterpolationMethod performance_interpolation_method ) {
			std::vector<double> target {evaporator_liquid_volumetric_flow_rate, evaporator_liquid_leaving_temperature, condenser_liquid_volumetric_flow_rate, condenser_liquid_entering_temperature, compressor_sequence_number};
			auto v = PerformanceMapBase::calculate_performance(target, performance_interpolation_method);
			LookupVariablesCoolingStruct s {v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8], };
			return s;
		}
		void from_json(const nlohmann::json& j, GridVariablesStandby& x) {
			a205_json_get<std::vector<double>>(j, *RS0001::logger, "environment_dry_bulb_temperature", x.environment_dry_bulb_temperature, x.environment_dry_bulb_temperature_is_set, true);
		}
		void GridVariablesStandby::populate_performance_map(PerformanceMapBase* performance_map) {
			add_grid_axis(performance_map, environment_dry_bulb_temperature);
			performance_map->finalize_grid(RS0001::logger);
		}
		const std::string_view GridVariablesStandby::environment_dry_bulb_temperature_units = "K";

		const std::string_view GridVariablesStandby::environment_dry_bulb_temperature_description = "Dry bulb temperature of the air in the environment of the chiller";

		const std::string_view GridVariablesStandby::environment_dry_bulb_temperature_name = "environment_dry_bulb_temperature";

		void from_json(const nlohmann::json& j, LookupVariablesStandby& x) {
			a205_json_get<std::vector<double>>(j, *RS0001::logger, "input_power", x.input_power, x.input_power_is_set, true);
		}
		void LookupVariablesStandby::populate_performance_map(PerformanceMapBase* performance_map) {
			add_data_table(performance_map, input_power);
		}
		const std::string_view LookupVariablesStandby::input_power_units = "W";

		const std::string_view LookupVariablesStandby::input_power_description = "Total power consumed in standby operation";

		const std::string_view LookupVariablesStandby::input_power_name = "input_power";

		void from_json(const nlohmann::json& j, PerformanceMapStandby& x) {
			a205_json_get<rs0001_ns::GridVariablesStandby>(j, *RS0001::logger, "grid_variables", x.grid_variables, x.grid_variables_is_set, true);
			x.grid_variables.populate_performance_map(&x);
			a205_json_get<rs0001_ns::LookupVariablesStandby>(j, *RS0001::logger, "lookup_variables", x.lookup_variables, x.lookup_variables_is_set, true);
			x.lookup_variables.populate_performance_map(&x);
		}
		void PerformanceMapStandby::initialize(const nlohmann::json& j) {
			a205_json_get<rs0001_ns::GridVariablesStandby>(j, *RS0001::logger, "grid_variables", grid_variables, grid_variables_is_set, true);
			grid_variables.populate_performance_map(this);
			a205_json_get<rs0001_ns::LookupVariablesStandby>(j, *RS0001::logger, "lookup_variables", lookup_variables, lookup_variables_is_set, true);
			lookup_variables.populate_performance_map(this);
		}
		const std::string_view PerformanceMapStandby::grid_variables_units = "";

		const std::string_view PerformanceMapStandby::lookup_variables_units = "";

		const std::string_view PerformanceMapStandby::grid_variables_description = "Data group defining the grid variables for standby performance";

		const std::string_view PerformanceMapStandby::lookup_variables_description = "Data group defining the lookup variables for standby performance";

		const std::string_view PerformanceMapStandby::grid_variables_name = "grid_variables";

		const std::string_view PerformanceMapStandby::lookup_variables_name = "lookup_variables";

		LookupVariablesStandbyStruct PerformanceMapStandby::calculate_performance(double environment_dry_bulb_temperature, Btwxt::InterpolationMethod performance_interpolation_method ) {
			std::vector<double> target {environment_dry_bulb_temperature};
			auto v = PerformanceMapBase::calculate_performance(target, performance_interpolation_method);
			LookupVariablesStandbyStruct s {v[0], };
			return s;
		}
		void from_json(const nlohmann::json& j, Performance& x) {
			a205_json_get<ashrae205_ns::LiquidMixture>(j, *RS0001::logger, "evaporator_liquid_type", x.evaporator_liquid_type, x.evaporator_liquid_type_is_set, true);
			a205_json_get<ashrae205_ns::LiquidMixture>(j, *RS0001::logger, "condenser_liquid_type", x.condenser_liquid_type, x.condenser_liquid_type_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "evaporator_fouling_factor", x.evaporator_fouling_factor, x.evaporator_fouling_factor_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "condenser_fouling_factor", x.condenser_fouling_factor, x.condenser_fouling_factor_is_set, true);
			a205_json_get<ashrae205_ns::SpeedControlType>(j, *RS0001::logger, "compressor_speed_control_type", x.compressor_speed_control_type, x.compressor_speed_control_type_is_set, true);
			a205_json_get<double>(j, *RS0001::logger, "maximum_power", x.maximum_power, x.maximum_power_is_set, false);
			a205_json_get<double>(j, *RS0001::logger, "cycling_degradation_coefficient", x.cycling_degradation_coefficient, x.cycling_degradation_coefficient_is_set, true);
			a205_json_get<rs0001_ns::PerformanceMapCooling>(j, *RS0001::logger, "performance_map_cooling", x.performance_map_cooling, x.performance_map_cooling_is_set, true);
			a205_json_get<rs0001_ns::PerformanceMapStandby>(j, *RS0001::logger, "performance_map_standby", x.performance_map_standby, x.performance_map_standby_is_set, true);
		}
		const std::string_view Performance::evaporator_liquid_type_units = "";

		const std::string_view Performance::condenser_liquid_type_units = "";

		const std::string_view Performance::evaporator_fouling_factor_units = "m2-K/W";

		const std::string_view Performance::condenser_fouling_factor_units = "m2-K/W";

		const std::string_view Performance::compressor_speed_control_type_units = "";

		const std::string_view Performance::maximum_power_units = "W";

		const std::string_view Performance::cycling_degradation_coefficient_units = "-";

		const std::string_view Performance::performance_map_cooling_units = "";

		const std::string_view Performance::performance_map_standby_units = "";

		const std::string_view Performance::evaporator_liquid_type_description = "Type of liquid in evaporator";

		const std::string_view Performance::condenser_liquid_type_description = "Type of liquid in condenser";

		const std::string_view Performance::evaporator_fouling_factor_description = "Factor of heat transfer inhibition due to heat exchanger fouling layer";

		const std::string_view Performance::condenser_fouling_factor_description = "Factor of heat transfer inhibition due to heat exchanger fouling layer";

		const std::string_view Performance::compressor_speed_control_type_description = "Type of compressor speed control";

		const std::string_view Performance::maximum_power_description = "Maximum input power at which the chiller operates reliably and continuously";

		const std::string_view Performance::cycling_degradation_coefficient_description = "Cycling degradation coefficient (C~D~) as described in AHRI 550/590 or AHRI 551/591";

		const std::string_view Performance::performance_map_cooling_description = "Data group describing cooling performance over a range of conditions";

		const std::string_view Performance::performance_map_standby_description = "Data group describing standby performance";

		const std::string_view Performance::evaporator_liquid_type_name = "evaporator_liquid_type";

		const std::string_view Performance::condenser_liquid_type_name = "condenser_liquid_type";

		const std::string_view Performance::evaporator_fouling_factor_name = "evaporator_fouling_factor";

		const std::string_view Performance::condenser_fouling_factor_name = "condenser_fouling_factor";

		const std::string_view Performance::compressor_speed_control_type_name = "compressor_speed_control_type";

		const std::string_view Performance::maximum_power_name = "maximum_power";

		const std::string_view Performance::cycling_degradation_coefficient_name = "cycling_degradation_coefficient";

		const std::string_view Performance::performance_map_cooling_name = "performance_map_cooling";

		const std::string_view Performance::performance_map_standby_name = "performance_map_standby";

		void from_json(const nlohmann::json& j, RS0001& x) {
			a205_json_get<ashrae205_ns::Metadata>(j, *RS0001::logger, "metadata", x.metadata, x.metadata_is_set, true);
			a205_json_get<rs0001_ns::Description>(j, *RS0001::logger, "description", x.description, x.description_is_set, false);
			a205_json_get<rs0001_ns::Performance>(j, *RS0001::logger, "performance", x.performance, x.performance_is_set, true);
		}
		void RS0001::initialize(const nlohmann::json& j) {
			a205_json_get<ashrae205_ns::Metadata>(j, *RS0001::logger, "metadata", metadata, metadata_is_set, true);
			a205_json_get<rs0001_ns::Description>(j, *RS0001::logger, "description", description, description_is_set, false);
			a205_json_get<rs0001_ns::Performance>(j, *RS0001::logger, "performance", performance, performance_is_set, true);
		}
		 std::shared_ptr<Courierr::Courierr> RS0001::logger {};

		const std::string_view RS0001::metadata_units = "";

		const std::string_view RS0001::description_units = "";

		const std::string_view RS0001::performance_units = "";

		const std::string_view RS0001::metadata_description = "Metadata data group";

		const std::string_view RS0001::description_description = "Data group describing product and rating information";

		const std::string_view RS0001::performance_description = "Data group containing performance information";

		const std::string_view RS0001::metadata_name = "metadata";

		const std::string_view RS0001::description_name = "description";

		const std::string_view RS0001::performance_name = "performance";

	}
}

