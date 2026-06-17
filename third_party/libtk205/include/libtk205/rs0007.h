#ifndef RS0007_H_
#define RS0007_H_
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

	namespace rs0007_ns  {
	
		enum class DriveType {
			V_BELT,
			COGGED_BELT,
			SYNCHRONOUS_BELT,
			GEAR,
			CHAIN,
			UNKNOWN
		};
		const static std::unordered_map<DriveType, enum_info> DriveType_info {
			{DriveType::V_BELT, {"V_BELT", "V-Belt", "V-Belt"}},
			{DriveType::COGGED_BELT, {"COGGED_BELT", "Cogged Belt", "Cogged Belt"}},
			{DriveType::SYNCHRONOUS_BELT, {"SYNCHRONOUS_BELT", "Synchronous Belt", "Synchronous Belt"}},
			{DriveType::GEAR, {"GEAR", "Gear", "Gear"}},
			{DriveType::CHAIN, {"CHAIN", "Chain", "Chain"}},
			{DriveType::UNKNOWN, {"UNKNOWN", "None","None"}}
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
			rs0007_ns::DriveType drive_type;
			bool manufacturer_is_set;
			bool model_number_is_set;
			bool drive_type_is_set;
			const static std::string_view manufacturer_units;
			const static std::string_view model_number_units;
			const static std::string_view drive_type_units;
			const static std::string_view manufacturer_description;
			const static std::string_view model_number_description;
			const static std::string_view drive_type_description;
			const static std::string_view manufacturer_name;
			const static std::string_view model_number_name;
			const static std::string_view drive_type_name;
		};
		class Description  {
		public:
			rs0007_ns::ProductInformation product_information;
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
				index_count
			};
			std::vector<double> output_power;
			bool output_power_is_set;
			const static std::string_view output_power_units;
			const static std::string_view output_power_description;
			const static std::string_view output_power_name;
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
			rs0007_ns::GridVariables grid_variables;
			rs0007_ns::LookupVariables lookup_variables;
			bool grid_variables_is_set;
			bool lookup_variables_is_set;
			const static std::string_view grid_variables_units;
			const static std::string_view lookup_variables_units;
			const static std::string_view grid_variables_description;
			const static std::string_view lookup_variables_description;
			const static std::string_view grid_variables_name;
			const static std::string_view lookup_variables_name;
			using PerformanceMapBase::calculate_performance;
			LookupVariablesStruct calculate_performance (double output_power, Btwxt::InterpolationMethod performance_interpolation_method = Btwxt::InterpolationMethod::linear);
		};
		class Performance  {
		public:
			double speed_ratio;
			rs0007_ns::PerformanceMap performance_map;
			bool speed_ratio_is_set;
			bool performance_map_is_set;
			const static std::string_view speed_ratio_units;
			const static std::string_view performance_map_units;
			const static std::string_view speed_ratio_description;
			const static std::string_view performance_map_description;
			const static std::string_view speed_ratio_name;
			const static std::string_view performance_map_name;
		};
		class RS0007  : public RSInstanceBase {
		public:
			void initialize (const nlohmann::json& j) override;
			static std::shared_ptr<Courierr::Courierr> logger;
			ashrae205_ns::Metadata metadata;
			rs0007_ns::Description description;
			rs0007_ns::Performance performance;
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
		NLOHMANN_JSON_SERIALIZE_ENUM (DriveType, {
			{DriveType::UNKNOWN, "UNKNOWN"},
			{DriveType::V_BELT, "V_BELT"},
			{DriveType::COGGED_BELT, "COGGED_BELT"},
			{DriveType::SYNCHRONOUS_BELT, "SYNCHRONOUS_BELT"},
			{DriveType::GEAR, "GEAR"},
			{DriveType::CHAIN, "CHAIN"},
		})
		void from_json (const nlohmann::json& j, RS0007& x);
		void from_json (const nlohmann::json& j, Description& x);
		void from_json (const nlohmann::json& j, ProductInformation& x);
		void from_json (const nlohmann::json& j, Performance& x);
		void from_json (const nlohmann::json& j, PerformanceMap& x);
		void from_json (const nlohmann::json& j, GridVariables& x);
		void from_json (const nlohmann::json& j, LookupVariables& x);
	}
}
#endif

