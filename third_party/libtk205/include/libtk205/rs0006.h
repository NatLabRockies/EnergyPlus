#ifndef RS0006_H_
#define RS0006_H_
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

	namespace rs0006_ns  {
	
		enum class CoolingMethod {
			PASSIVE_COOLED,
			ACTIVE_AIR_COOLED,
			ACTIVE_LIQUID_COOLED,
			UNKNOWN
		};
		const static std::unordered_map<CoolingMethod, enum_info> CoolingMethod_info {
			{CoolingMethod::PASSIVE_COOLED, {"PASSIVE_COOLED", "Passive Cooled", "Drive is cooled using natural air convection within the surrounding environment"}},
			{CoolingMethod::ACTIVE_AIR_COOLED, {"ACTIVE_AIR_COOLED", "Active Air Cooled", "Drive is cooled using forced air convection within the surrounding environment"}},
			{CoolingMethod::ACTIVE_LIQUID_COOLED, {"ACTIVE_LIQUID_COOLED", "Active Liquid Cooled", "Drive is cooled using forced liquid convection, transferring heat to the liquid"}},
			{CoolingMethod::UNKNOWN, {"UNKNOWN", "None","None"}}
		};
		class Schema  {
		public:
			const static std::string_view schema_title;
			const static std::string_view schema_version;
			const static std::string_view schema_description;
		};
		class ProductInformation  {
		public:
			std::string manufacturer;
			ashrae205_ns::Pattern model_number;
			bool manufacturer_is_set;
			bool model_number_is_set;
			const static std::string_view manufacturer_units;
			const static std::string_view model_number_units;
			const static std::string_view manufacturer_description;
			const static std::string_view model_number_description;
			const static std::string_view manufacturer_name;
			const static std::string_view model_number_name;
		};
		class Description  {
		public:
			rs0006_ns::ProductInformation product_information;
			bool product_information_is_set;
			const static std::string_view product_information_units;
			const static std::string_view product_information_description;
			const static std::string_view product_information_name;
		};
		class GridVariables  : public GridVariablesBase {
		public:
			void populate_performance_map (PerformanceMapBase* performance_map) override;
			enum  {
				output_power_index,
				output_frequency_index,
				index_count
			};
			std::vector<double> output_power;
			std::vector<double> output_frequency;
			bool output_power_is_set;
			bool output_frequency_is_set;
			const static std::string_view output_power_units;
			const static std::string_view output_frequency_units;
			const static std::string_view output_power_description;
			const static std::string_view output_frequency_description;
			const static std::string_view output_power_name;
			const static std::string_view output_frequency_name;
		};
		struct LookupVariables  : public LookupVariablesBase {
		
			void populate_performance_map (PerformanceMapBase* performance_map) override;
			enum  {
				efficiency_index,
				index_count
			};
			std::vector<double> efficiency;
			bool efficiency_is_set;
			const static std::string_view efficiency_units;
			const static std::string_view efficiency_description;
			const static std::string_view efficiency_name;
		};
		struct LookupVariablesStruct {
			double efficiency;
		};
		class PerformanceMap  : public PerformanceMapBase {
		public:
			void initialize (const nlohmann::json& j) override;
			rs0006_ns::GridVariables grid_variables;
			rs0006_ns::LookupVariables lookup_variables;
			bool grid_variables_is_set;
			bool lookup_variables_is_set;
			const static std::string_view grid_variables_units;
			const static std::string_view lookup_variables_units;
			const static std::string_view grid_variables_description;
			const static std::string_view lookup_variables_description;
			const static std::string_view grid_variables_name;
			const static std::string_view lookup_variables_name;
			using PerformanceMapBase::calculate_performance;
			LookupVariablesStruct calculate_performance (double output_power, double output_frequency, Btwxt::InterpolationMethod performance_interpolation_method = Btwxt::InterpolationMethod::linear);
		};
		class Performance  {
		public:
			double maximum_power;
			double standby_power;
			rs0006_ns::CoolingMethod cooling_method;
			rs0006_ns::PerformanceMap performance_map;
			bool maximum_power_is_set;
			bool standby_power_is_set;
			bool cooling_method_is_set;
			bool performance_map_is_set;
			const static std::string_view maximum_power_units;
			const static std::string_view standby_power_units;
			const static std::string_view cooling_method_units;
			const static std::string_view performance_map_units;
			const static std::string_view maximum_power_description;
			const static std::string_view standby_power_description;
			const static std::string_view cooling_method_description;
			const static std::string_view performance_map_description;
			const static std::string_view maximum_power_name;
			const static std::string_view standby_power_name;
			const static std::string_view cooling_method_name;
			const static std::string_view performance_map_name;
		};
		class RS0006  : public RSInstanceBase {
		public:
			void initialize (const nlohmann::json& j) override;
			static std::shared_ptr<Courierr::Courierr> logger;
			ashrae205_ns::Metadata metadata;
			rs0006_ns::Description description;
			rs0006_ns::Performance performance;
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
		NLOHMANN_JSON_SERIALIZE_ENUM (CoolingMethod, {
			{CoolingMethod::UNKNOWN, "UNKNOWN"},
			{CoolingMethod::PASSIVE_COOLED, "PASSIVE_COOLED"},
			{CoolingMethod::ACTIVE_AIR_COOLED, "ACTIVE_AIR_COOLED"},
			{CoolingMethod::ACTIVE_LIQUID_COOLED, "ACTIVE_LIQUID_COOLED"},
		})
		void from_json (const nlohmann::json& j, RS0006& x);
		void from_json (const nlohmann::json& j, Description& x);
		void from_json (const nlohmann::json& j, ProductInformation& x);
		void from_json (const nlohmann::json& j, Performance& x);
		void from_json (const nlohmann::json& j, PerformanceMap& x);
		void from_json (const nlohmann::json& j, GridVariables& x);
		void from_json (const nlohmann::json& j, LookupVariables& x);
	}
}
#endif

