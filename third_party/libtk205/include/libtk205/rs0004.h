#ifndef RS0004_H_
#define RS0004_H_
#include <ashrae205.h>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <typeinfo_205.h>
#include <courierr/courierr.h>
#include <rs_instance_base.h>
#include <performance_map_base.h>
#include <grid_variables_base.h>
#include <lookup_variables_base.h>

/// @note  This class has been auto-generated. Local changes will not be saved!

namespace tk205  {

	namespace rs0004_ns  {
	
		class Schema  {
		public:
			const static std::string_view schema_title;
			const static std::string_view schema_version;
			const static std::string_view schema_description;
		};
		class ProductInformation  {
		public:
			std::string outdoor_unit_manufacturer;
			ashrae205_ns::Pattern outdoor_unit_model_number;
			std::string indoor_unit_manufacturer;
			ashrae205_ns::Pattern indoor_unit_model_number;
			std::string refrigerant;
			ashrae205_ns::CompressorType compressor_type;
			bool outdoor_unit_manufacturer_is_set;
			bool outdoor_unit_model_number_is_set;
			bool indoor_unit_manufacturer_is_set;
			bool indoor_unit_model_number_is_set;
			bool refrigerant_is_set;
			bool compressor_type_is_set;
			const static std::string_view outdoor_unit_manufacturer_units;
			const static std::string_view outdoor_unit_model_number_units;
			const static std::string_view indoor_unit_manufacturer_units;
			const static std::string_view indoor_unit_model_number_units;
			const static std::string_view refrigerant_units;
			const static std::string_view compressor_type_units;
			const static std::string_view outdoor_unit_manufacturer_description;
			const static std::string_view outdoor_unit_model_number_description;
			const static std::string_view indoor_unit_manufacturer_description;
			const static std::string_view indoor_unit_model_number_description;
			const static std::string_view refrigerant_description;
			const static std::string_view compressor_type_description;
			const static std::string_view outdoor_unit_manufacturer_name;
			const static std::string_view outdoor_unit_model_number_name;
			const static std::string_view indoor_unit_manufacturer_name;
			const static std::string_view indoor_unit_model_number_name;
			const static std::string_view refrigerant_name;
			const static std::string_view compressor_type_name;
		};
		class Description  {
		public:
			rs0004_ns::ProductInformation product_information;
			bool product_information_is_set;
			const static std::string_view product_information_units;
			const static std::string_view product_information_description;
			const static std::string_view product_information_name;
		};
		class GridVariablesCooling  : public GridVariablesBase {
		public:
			void populate_performance_map (PerformanceMapBase* performance_map) override;
			enum  {
				outdoor_coil_entering_dry_bulb_temperature_index,
				indoor_coil_entering_relative_humidity_index,
				indoor_coil_entering_dry_bulb_temperature_index,
				indoor_coil_air_mass_flow_rate_index,
				compressor_sequence_number_index,
				ambient_absolute_air_pressure_index,
				index_count
			};
			std::vector<double> outdoor_coil_entering_dry_bulb_temperature;
			std::vector<double> indoor_coil_entering_relative_humidity;
			std::vector<double> indoor_coil_entering_dry_bulb_temperature;
			std::vector<double> indoor_coil_air_mass_flow_rate;
			std::vector<int> compressor_sequence_number;
			std::vector<double> ambient_absolute_air_pressure;
			bool outdoor_coil_entering_dry_bulb_temperature_is_set;
			bool indoor_coil_entering_relative_humidity_is_set;
			bool indoor_coil_entering_dry_bulb_temperature_is_set;
			bool indoor_coil_air_mass_flow_rate_is_set;
			bool compressor_sequence_number_is_set;
			bool ambient_absolute_air_pressure_is_set;
			const static std::string_view outdoor_coil_entering_dry_bulb_temperature_units;
			const static std::string_view indoor_coil_entering_relative_humidity_units;
			const static std::string_view indoor_coil_entering_dry_bulb_temperature_units;
			const static std::string_view indoor_coil_air_mass_flow_rate_units;
			const static std::string_view compressor_sequence_number_units;
			const static std::string_view ambient_absolute_air_pressure_units;
			const static std::string_view outdoor_coil_entering_dry_bulb_temperature_description;
			const static std::string_view indoor_coil_entering_relative_humidity_description;
			const static std::string_view indoor_coil_entering_dry_bulb_temperature_description;
			const static std::string_view indoor_coil_air_mass_flow_rate_description;
			const static std::string_view compressor_sequence_number_description;
			const static std::string_view ambient_absolute_air_pressure_description;
			const static std::string_view outdoor_coil_entering_dry_bulb_temperature_name;
			const static std::string_view indoor_coil_entering_relative_humidity_name;
			const static std::string_view indoor_coil_entering_dry_bulb_temperature_name;
			const static std::string_view indoor_coil_air_mass_flow_rate_name;
			const static std::string_view compressor_sequence_number_name;
			const static std::string_view ambient_absolute_air_pressure_name;
		};
		struct LookupVariablesCooling  : public LookupVariablesBase {
		
			void populate_performance_map (PerformanceMapBase* performance_map) override;
			enum  {
				gross_total_capacity_index,
				gross_sensible_capacity_index,
				gross_power_index,
				index_count
			};
			std::vector<double> gross_total_capacity;
			std::vector<double> gross_sensible_capacity;
			std::vector<double> gross_power;
			bool gross_total_capacity_is_set;
			bool gross_sensible_capacity_is_set;
			bool gross_power_is_set;
			const static std::string_view gross_total_capacity_units;
			const static std::string_view gross_sensible_capacity_units;
			const static std::string_view gross_power_units;
			const static std::string_view gross_total_capacity_description;
			const static std::string_view gross_sensible_capacity_description;
			const static std::string_view gross_power_description;
			const static std::string_view gross_total_capacity_name;
			const static std::string_view gross_sensible_capacity_name;
			const static std::string_view gross_power_name;
		};
		struct LookupVariablesCoolingStruct {
			double gross_total_capacity;
			double gross_sensible_capacity;
			double gross_power;
		};
		class PerformanceMapCooling  : public PerformanceMapBase {
		public:
			void initialize (const nlohmann::json& j) override;
			rs0004_ns::GridVariablesCooling grid_variables;
			rs0004_ns::LookupVariablesCooling lookup_variables;
			bool grid_variables_is_set;
			bool lookup_variables_is_set;
			const static std::string_view grid_variables_units;
			const static std::string_view lookup_variables_units;
			const static std::string_view grid_variables_description;
			const static std::string_view lookup_variables_description;
			const static std::string_view grid_variables_name;
			const static std::string_view lookup_variables_name;
			using PerformanceMapBase::calculate_performance;
			LookupVariablesCoolingStruct calculate_performance (double outdoor_coil_entering_dry_bulb_temperature, double indoor_coil_entering_relative_humidity, double indoor_coil_entering_dry_bulb_temperature, double indoor_coil_air_mass_flow_rate, double compressor_sequence_number, double ambient_absolute_air_pressure, Btwxt::InterpolationMethod performance_interpolation_method = Btwxt::InterpolationMethod::linear);
		};
		class GridVariablesStandby  : public GridVariablesBase {
		public:
			void populate_performance_map (PerformanceMapBase* performance_map) override;
			enum  {
				outdoor_coil_environment_dry_bulb_temperature_index,
				index_count
			};
			std::vector<double> outdoor_coil_environment_dry_bulb_temperature;
			bool outdoor_coil_environment_dry_bulb_temperature_is_set;
			const static std::string_view outdoor_coil_environment_dry_bulb_temperature_units;
			const static std::string_view outdoor_coil_environment_dry_bulb_temperature_description;
			const static std::string_view outdoor_coil_environment_dry_bulb_temperature_name;
		};
		struct LookupVariablesStandby  : public LookupVariablesBase {
		
			void populate_performance_map (PerformanceMapBase* performance_map) override;
			enum  {
				gross_power_index,
				index_count
			};
			std::vector<double> gross_power;
			bool gross_power_is_set;
			const static std::string_view gross_power_units;
			const static std::string_view gross_power_description;
			const static std::string_view gross_power_name;
		};
		struct LookupVariablesStandbyStruct {
			double gross_power;
		};
		class PerformanceMapStandby  : public PerformanceMapBase {
		public:
			void initialize (const nlohmann::json& j) override;
			rs0004_ns::GridVariablesStandby grid_variables;
			rs0004_ns::LookupVariablesStandby lookup_variables;
			bool grid_variables_is_set;
			bool lookup_variables_is_set;
			const static std::string_view grid_variables_units;
			const static std::string_view lookup_variables_units;
			const static std::string_view grid_variables_description;
			const static std::string_view lookup_variables_description;
			const static std::string_view grid_variables_name;
			const static std::string_view lookup_variables_name;
			using PerformanceMapBase::calculate_performance;
			LookupVariablesStandbyStruct calculate_performance (double outdoor_coil_environment_dry_bulb_temperature, Btwxt::InterpolationMethod performance_interpolation_method = Btwxt::InterpolationMethod::linear);
		};
		class Performance  {
		public:
			ashrae205_ns::SpeedControlType compressor_speed_control_type;
			double cycling_degradation_coefficient;
			rs0004_ns::PerformanceMapCooling performance_map_cooling;
			rs0004_ns::PerformanceMapStandby performance_map_standby;
			bool compressor_speed_control_type_is_set;
			bool cycling_degradation_coefficient_is_set;
			bool performance_map_cooling_is_set;
			bool performance_map_standby_is_set;
			const static std::string_view compressor_speed_control_type_units;
			const static std::string_view cycling_degradation_coefficient_units;
			const static std::string_view performance_map_cooling_units;
			const static std::string_view performance_map_standby_units;
			const static std::string_view compressor_speed_control_type_description;
			const static std::string_view cycling_degradation_coefficient_description;
			const static std::string_view performance_map_cooling_description;
			const static std::string_view performance_map_standby_description;
			const static std::string_view compressor_speed_control_type_name;
			const static std::string_view cycling_degradation_coefficient_name;
			const static std::string_view performance_map_cooling_name;
			const static std::string_view performance_map_standby_name;
		};
		class RS0004  : public RSInstanceBase {
		public:
			void initialize (const nlohmann::json& j) override;
			static std::shared_ptr<Courierr::Courierr> logger;
			ashrae205_ns::Metadata metadata;
			rs0004_ns::Description description;
			rs0004_ns::Performance performance;
			bool metadata_is_set;
			bool description_is_set;
			bool performance_is_set;
			const static std::string_view metadata_units;
			const static std::string_view description_units;
			const static std::string_view performance_units;
			const static std::string_view metadata_description;
			const static std::string_view description_description;
			const static std::string_view performance_description;
			const static std::string_view metadata_name;
			const static std::string_view description_name;
			const static std::string_view performance_name;
		};
		void from_json (const nlohmann::json& j, RS0004& x);
		void from_json (const nlohmann::json& j, Description& x);
		void from_json (const nlohmann::json& j, ProductInformation& x);
		void from_json (const nlohmann::json& j, Performance& x);
		void from_json (const nlohmann::json& j, PerformanceMapCooling& x);
		void from_json (const nlohmann::json& j, GridVariablesCooling& x);
		void from_json (const nlohmann::json& j, LookupVariablesCooling& x);
		void from_json (const nlohmann::json& j, PerformanceMapStandby& x);
		void from_json (const nlohmann::json& j, GridVariablesStandby& x);
		void from_json (const nlohmann::json& j, LookupVariablesStandby& x);
	}
}
#endif

