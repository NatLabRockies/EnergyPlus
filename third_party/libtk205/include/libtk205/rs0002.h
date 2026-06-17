#ifndef RS0002_H_
#define RS0002_H_
#include <ashrae205.h>
#include <rs0003.h>
#include <rs0004.h>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <typeinfo_205.h>
#include <courierr/courierr.h>
#include <rs_instance_base.h>

/// @note  This class has been auto-generated. Local changes will not be saved!

namespace tk205  {

	namespace rs0002_ns  {
	
		enum class FanPosition {
			BLOW_THROUGH,
			DRAW_THROUGH,
			UNKNOWN
		};
		const static std::unordered_map<FanPosition, enum_info> FanPosition_info {
			{FanPosition::BLOW_THROUGH, {"BLOW_THROUGH", "Blow Through", "Fan is placed upstream of the indoor coil"}},
			{FanPosition::DRAW_THROUGH, {"DRAW_THROUGH", "Draw Through", "Fan is placed downstream of the indoor coil"}},
			{FanPosition::UNKNOWN, {"UNKNOWN", "None","None"}}
		};
		enum class AHRI210240TestStandardYear {
			IP_2008,
			IP_2017,
			IP_2023,
			UNKNOWN
		};
		const static std::unordered_map<AHRI210240TestStandardYear, enum_info> AHRI210240TestStandardYear_info {
			{AHRI210240TestStandardYear::IP_2008, {"IP_2008", "2008", "Rating is based on 2008 AHRI standard[@AHRI2102008]"}},
			{AHRI210240TestStandardYear::IP_2017, {"IP_2017", "2017", "Rating is based on 2017 AHRI standard[@AHRI2102017]"}},
			{AHRI210240TestStandardYear::IP_2023, {"IP_2023", "2023", "Rating is based on 2023 AHRI standard[@AHRI2102023]"}},
			{AHRI210240TestStandardYear::UNKNOWN, {"UNKNOWN", "None","None"}}
		};
		enum class AHRI210240CompressorStagingType {
			SINGLE_STAGE,
			TWO_STAGE,
			VARIABLE_STAGE,
			UNKNOWN
		};
		const static std::unordered_map<AHRI210240CompressorStagingType, enum_info> AHRI210240CompressorStagingType_info {
			{AHRI210240CompressorStagingType::SINGLE_STAGE, {"SINGLE_STAGE", "Single Stage", "Single, fixed capacity compressor"}},
			{AHRI210240CompressorStagingType::TWO_STAGE, {"TWO_STAGE", "Two-Stage", "Compressor or group of compressors operating with only two stages of capacity"}},
			{AHRI210240CompressorStagingType::VARIABLE_STAGE, {"VARIABLE_STAGE", "Variable Stage", "Variable speed compressor or compressor or group of compressors with three or more stages of capacity"}},
			{AHRI210240CompressorStagingType::UNKNOWN, {"UNKNOWN", "None","None"}}
		};
		enum class AHRI340360TestStandardYear {
			IP_2007,
			IP_2015,
			IP_2019,
			UNKNOWN
		};
		const static std::unordered_map<AHRI340360TestStandardYear, enum_info> AHRI340360TestStandardYear_info {
			{AHRI340360TestStandardYear::IP_2007, {"IP_2007", "2007", "Rating is based on 2007 AHRI standard[@AHRI3402007]"}},
			{AHRI340360TestStandardYear::IP_2015, {"IP_2015", "2015", "Rating is based on 2015 AHRI standard[@AHRI3402015]"}},
			{AHRI340360TestStandardYear::IP_2019, {"IP_2019", "2019", "Rating is based on 2019 AHRI standard[@AHRI3402019]"}},
			{AHRI340360TestStandardYear::UNKNOWN, {"UNKNOWN", "None","None"}}
		};
		enum class AHRI340360CapacityControlType {
			FIXED_CAPACITY,
			STAGED_CAPACITY,
			PROPORTIONAL_CAPACITY,
			UNKNOWN
		};
		const static std::unordered_map<AHRI340360CapacityControlType, enum_info> AHRI340360CapacityControlType_info {
			{AHRI340360CapacityControlType::FIXED_CAPACITY, {"FIXED_CAPACITY", "Fixed Capacity", "Limited to a single stage of refrigeration capacity"}},
			{AHRI340360CapacityControlType::STAGED_CAPACITY, {"STAGED_CAPACITY", "Staged Capacity", "Limited to multiple fixed or discrete stages of refrigeration capacity"}},
			{AHRI340360CapacityControlType::PROPORTIONAL_CAPACITY, {"PROPORTIONAL_CAPACITY", "Proportional Capacity", "Compressor capacity can be modulated continuously or in steps not more than 5% of the rated capacity"}},
			{AHRI340360CapacityControlType::UNKNOWN, {"UNKNOWN", "None","None"}}
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
		class RatingAHRI210240  {
		public:
			std::string certified_reference_number;
			rs0002_ns::AHRI210240TestStandardYear test_standard_year;
			std::string rating_source;
			rs0002_ns::AHRI210240CompressorStagingType staging_type;
			double seer;
			double eer_a_full;
			double eer_b_full;
			double cooling_a_full_capacity;
			double cooling_b_full_capacity;
			double cooling_b_low_capacity;
			double cooling_f_low_capacity;
			double cooling_g_low_capacity;
			double cooling_i_low_capacity;
			double cooling_a_full_power;
			double cooling_b_full_power;
			double cooling_b_low_power;
			double cooling_f_low_power;
			double cooling_g_low_power;
			double cooling_i_low_power;
			double cooling_full_fan_power;
			double cooling_full_air_volumetric_flow_rate;
			double cooling_low_fan_power;
			double cooling_low_air_volumetric_flow_rate;
			bool rating_recalculatable_from_performance_data;
			std::string rating_recalculatable_explanation;
			bool certified_reference_number_is_set;
			bool test_standard_year_is_set;
			bool rating_source_is_set;
			bool staging_type_is_set;
			bool seer_is_set;
			bool eer_a_full_is_set;
			bool eer_b_full_is_set;
			bool cooling_a_full_capacity_is_set;
			bool cooling_b_full_capacity_is_set;
			bool cooling_b_low_capacity_is_set;
			bool cooling_f_low_capacity_is_set;
			bool cooling_g_low_capacity_is_set;
			bool cooling_i_low_capacity_is_set;
			bool cooling_a_full_power_is_set;
			bool cooling_b_full_power_is_set;
			bool cooling_b_low_power_is_set;
			bool cooling_f_low_power_is_set;
			bool cooling_g_low_power_is_set;
			bool cooling_i_low_power_is_set;
			bool cooling_full_fan_power_is_set;
			bool cooling_full_air_volumetric_flow_rate_is_set;
			bool cooling_low_fan_power_is_set;
			bool cooling_low_air_volumetric_flow_rate_is_set;
			bool rating_recalculatable_from_performance_data_is_set;
			bool rating_recalculatable_explanation_is_set;
			const static std::string_view certified_reference_number_units;
			const static std::string_view test_standard_year_units;
			const static std::string_view rating_source_units;
			const static std::string_view staging_type_units;
			const static std::string_view seer_units;
			const static std::string_view eer_a_full_units;
			const static std::string_view eer_b_full_units;
			const static std::string_view cooling_a_full_capacity_units;
			const static std::string_view cooling_b_full_capacity_units;
			const static std::string_view cooling_b_low_capacity_units;
			const static std::string_view cooling_f_low_capacity_units;
			const static std::string_view cooling_g_low_capacity_units;
			const static std::string_view cooling_i_low_capacity_units;
			const static std::string_view cooling_a_full_power_units;
			const static std::string_view cooling_b_full_power_units;
			const static std::string_view cooling_b_low_power_units;
			const static std::string_view cooling_f_low_power_units;
			const static std::string_view cooling_g_low_power_units;
			const static std::string_view cooling_i_low_power_units;
			const static std::string_view cooling_full_fan_power_units;
			const static std::string_view cooling_full_air_volumetric_flow_rate_units;
			const static std::string_view cooling_low_fan_power_units;
			const static std::string_view cooling_low_air_volumetric_flow_rate_units;
			const static std::string_view rating_recalculatable_from_performance_data_units;
			const static std::string_view rating_recalculatable_explanation_units;
			const static std::string_view certified_reference_number_description;
			const static std::string_view test_standard_year_description;
			const static std::string_view rating_source_description;
			const static std::string_view staging_type_description;
			const static std::string_view seer_description;
			const static std::string_view eer_a_full_description;
			const static std::string_view eer_b_full_description;
			const static std::string_view cooling_a_full_capacity_description;
			const static std::string_view cooling_b_full_capacity_description;
			const static std::string_view cooling_b_low_capacity_description;
			const static std::string_view cooling_f_low_capacity_description;
			const static std::string_view cooling_g_low_capacity_description;
			const static std::string_view cooling_i_low_capacity_description;
			const static std::string_view cooling_a_full_power_description;
			const static std::string_view cooling_b_full_power_description;
			const static std::string_view cooling_b_low_power_description;
			const static std::string_view cooling_f_low_power_description;
			const static std::string_view cooling_g_low_power_description;
			const static std::string_view cooling_i_low_power_description;
			const static std::string_view cooling_full_fan_power_description;
			const static std::string_view cooling_full_air_volumetric_flow_rate_description;
			const static std::string_view cooling_low_fan_power_description;
			const static std::string_view cooling_low_air_volumetric_flow_rate_description;
			const static std::string_view rating_recalculatable_from_performance_data_description;
			const static std::string_view rating_recalculatable_explanation_description;
			const static std::string_view certified_reference_number_name;
			const static std::string_view test_standard_year_name;
			const static std::string_view rating_source_name;
			const static std::string_view staging_type_name;
			const static std::string_view seer_name;
			const static std::string_view eer_a_full_name;
			const static std::string_view eer_b_full_name;
			const static std::string_view cooling_a_full_capacity_name;
			const static std::string_view cooling_b_full_capacity_name;
			const static std::string_view cooling_b_low_capacity_name;
			const static std::string_view cooling_f_low_capacity_name;
			const static std::string_view cooling_g_low_capacity_name;
			const static std::string_view cooling_i_low_capacity_name;
			const static std::string_view cooling_a_full_power_name;
			const static std::string_view cooling_b_full_power_name;
			const static std::string_view cooling_b_low_power_name;
			const static std::string_view cooling_f_low_power_name;
			const static std::string_view cooling_g_low_power_name;
			const static std::string_view cooling_i_low_power_name;
			const static std::string_view cooling_full_fan_power_name;
			const static std::string_view cooling_full_air_volumetric_flow_rate_name;
			const static std::string_view cooling_low_fan_power_name;
			const static std::string_view cooling_low_air_volumetric_flow_rate_name;
			const static std::string_view rating_recalculatable_from_performance_data_name;
			const static std::string_view rating_recalculatable_explanation_name;
		};
		class RatingAHRI340360CoolingPartLoadPoint  {
		public:
			double capacity;
			double net_power;
			double indoor_fan_power;
			double auxiliary_power;
			double air_volumetric_flow_rate;
			bool capacity_is_set;
			bool net_power_is_set;
			bool indoor_fan_power_is_set;
			bool auxiliary_power_is_set;
			bool air_volumetric_flow_rate_is_set;
			const static std::string_view capacity_units;
			const static std::string_view net_power_units;
			const static std::string_view indoor_fan_power_units;
			const static std::string_view auxiliary_power_units;
			const static std::string_view air_volumetric_flow_rate_units;
			const static std::string_view capacity_description;
			const static std::string_view net_power_description;
			const static std::string_view indoor_fan_power_description;
			const static std::string_view auxiliary_power_description;
			const static std::string_view air_volumetric_flow_rate_description;
			const static std::string_view capacity_name;
			const static std::string_view net_power_name;
			const static std::string_view indoor_fan_power_name;
			const static std::string_view auxiliary_power_name;
			const static std::string_view air_volumetric_flow_rate_name;
		};
		class RatingAHRI340360  {
		public:
			std::string certified_reference_number;
			rs0002_ns::AHRI340360TestStandardYear test_standard_year;
			std::string rating_source;
			rs0002_ns::AHRI340360CapacityControlType capacity_control_type;
			double ieer;
			double eer;
			double cooling_capacity;
			std::vector<rs0002_ns::RatingAHRI340360CoolingPartLoadPoint> part_load_points;
			bool rating_recalculatable_from_performance_data;
			std::string rating_recalculatable_explanation;
			bool certified_reference_number_is_set;
			bool test_standard_year_is_set;
			bool rating_source_is_set;
			bool capacity_control_type_is_set;
			bool ieer_is_set;
			bool eer_is_set;
			bool cooling_capacity_is_set;
			bool part_load_points_is_set;
			bool rating_recalculatable_from_performance_data_is_set;
			bool rating_recalculatable_explanation_is_set;
			const static std::string_view certified_reference_number_units;
			const static std::string_view test_standard_year_units;
			const static std::string_view rating_source_units;
			const static std::string_view capacity_control_type_units;
			const static std::string_view ieer_units;
			const static std::string_view eer_units;
			const static std::string_view cooling_capacity_units;
			const static std::string_view part_load_points_units;
			const static std::string_view rating_recalculatable_from_performance_data_units;
			const static std::string_view rating_recalculatable_explanation_units;
			const static std::string_view certified_reference_number_description;
			const static std::string_view test_standard_year_description;
			const static std::string_view rating_source_description;
			const static std::string_view capacity_control_type_description;
			const static std::string_view ieer_description;
			const static std::string_view eer_description;
			const static std::string_view cooling_capacity_description;
			const static std::string_view part_load_points_description;
			const static std::string_view rating_recalculatable_from_performance_data_description;
			const static std::string_view rating_recalculatable_explanation_description;
			const static std::string_view certified_reference_number_name;
			const static std::string_view test_standard_year_name;
			const static std::string_view rating_source_name;
			const static std::string_view capacity_control_type_name;
			const static std::string_view ieer_name;
			const static std::string_view eer_name;
			const static std::string_view cooling_capacity_name;
			const static std::string_view part_load_points_name;
			const static std::string_view rating_recalculatable_from_performance_data_name;
			const static std::string_view rating_recalculatable_explanation_name;
		};
		class Description  {
		public:
			rs0002_ns::ProductInformation product_information;
			rs0002_ns::RatingAHRI210240 rating_ahri_210_240;
			rs0002_ns::RatingAHRI340360 rating_ahri_340_360;
			bool product_information_is_set;
			bool rating_ahri_210_240_is_set;
			bool rating_ahri_340_360_is_set;
			const static std::string_view product_information_units;
			const static std::string_view rating_ahri_210_240_units;
			const static std::string_view rating_ahri_340_360_units;
			const static std::string_view product_information_description;
			const static std::string_view rating_ahri_210_240_description;
			const static std::string_view rating_ahri_340_360_description;
			const static std::string_view product_information_name;
			const static std::string_view rating_ahri_210_240_name;
			const static std::string_view rating_ahri_340_360_name;
		};
		class Performance  {
		public:
			double standby_power;
			rs0003_ns::RS0003 indoor_fan_representation;
			rs0002_ns::FanPosition fan_position;
			rs0004_ns::RS0004 dx_system_representation;
			bool standby_power_is_set;
			bool indoor_fan_representation_is_set;
			bool fan_position_is_set;
			bool dx_system_representation_is_set;
			const static std::string_view standby_power_units;
			const static std::string_view indoor_fan_representation_units;
			const static std::string_view fan_position_units;
			const static std::string_view dx_system_representation_units;
			const static std::string_view standby_power_description;
			const static std::string_view indoor_fan_representation_description;
			const static std::string_view fan_position_description;
			const static std::string_view dx_system_representation_description;
			const static std::string_view standby_power_name;
			const static std::string_view indoor_fan_representation_name;
			const static std::string_view fan_position_name;
			const static std::string_view dx_system_representation_name;
		};
		class RS0002  : public RSInstanceBase {
		public:
			void initialize (const nlohmann::json& j) override;
			static std::shared_ptr<Courierr::Courierr> logger;
			ashrae205_ns::Metadata metadata;
			rs0002_ns::Description description;
			rs0002_ns::Performance performance;
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
		NLOHMANN_JSON_SERIALIZE_ENUM (FanPosition, {
			{FanPosition::UNKNOWN, "UNKNOWN"},
			{FanPosition::BLOW_THROUGH, "BLOW_THROUGH"},
			{FanPosition::DRAW_THROUGH, "DRAW_THROUGH"},
		})
		NLOHMANN_JSON_SERIALIZE_ENUM (AHRI210240TestStandardYear, {
			{AHRI210240TestStandardYear::UNKNOWN, "UNKNOWN"},
			{AHRI210240TestStandardYear::IP_2008, "IP_2008"},
			{AHRI210240TestStandardYear::IP_2017, "IP_2017"},
			{AHRI210240TestStandardYear::IP_2023, "IP_2023"},
		})
		NLOHMANN_JSON_SERIALIZE_ENUM (AHRI210240CompressorStagingType, {
			{AHRI210240CompressorStagingType::UNKNOWN, "UNKNOWN"},
			{AHRI210240CompressorStagingType::SINGLE_STAGE, "SINGLE_STAGE"},
			{AHRI210240CompressorStagingType::TWO_STAGE, "TWO_STAGE"},
			{AHRI210240CompressorStagingType::VARIABLE_STAGE, "VARIABLE_STAGE"},
		})
		NLOHMANN_JSON_SERIALIZE_ENUM (AHRI340360TestStandardYear, {
			{AHRI340360TestStandardYear::UNKNOWN, "UNKNOWN"},
			{AHRI340360TestStandardYear::IP_2007, "IP_2007"},
			{AHRI340360TestStandardYear::IP_2015, "IP_2015"},
			{AHRI340360TestStandardYear::IP_2019, "IP_2019"},
		})
		NLOHMANN_JSON_SERIALIZE_ENUM (AHRI340360CapacityControlType, {
			{AHRI340360CapacityControlType::UNKNOWN, "UNKNOWN"},
			{AHRI340360CapacityControlType::FIXED_CAPACITY, "FIXED_CAPACITY"},
			{AHRI340360CapacityControlType::STAGED_CAPACITY, "STAGED_CAPACITY"},
			{AHRI340360CapacityControlType::PROPORTIONAL_CAPACITY, "PROPORTIONAL_CAPACITY"},
		})
		void from_json (const nlohmann::json& j, RS0002& x);
		void from_json (const nlohmann::json& j, Description& x);
		void from_json (const nlohmann::json& j, ProductInformation& x);
		void from_json (const nlohmann::json& j, RatingAHRI210240& x);
		void from_json (const nlohmann::json& j, RatingAHRI340360& x);
		void from_json (const nlohmann::json& j, RatingAHRI340360CoolingPartLoadPoint& x);
		void from_json (const nlohmann::json& j, Performance& x);
	}
}
#endif

