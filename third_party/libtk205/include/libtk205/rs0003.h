#ifndef RS0003_H_
#define RS0003_H_
#include <ashrae205.h>
#include <rs0005.h>
#include <rs0007.h>
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

	namespace rs0003_ns  {
	
		enum class InstallationSpeedControlType {
			FIXED,
			VARIABLE,
			UNKNOWN
		};
		const static std::unordered_map<InstallationSpeedControlType, enum_info> InstallationSpeedControlType_info {
			{InstallationSpeedControlType::FIXED, {"FIXED", "Fixed", "Fan speed does not change after installation"}},
			{InstallationSpeedControlType::VARIABLE, {"VARIABLE", "Variable", "Fan speed can change depending on operation after installation"}},
			{InstallationSpeedControlType::UNKNOWN, {"UNKNOWN", "None","None"}}
		};
		enum class ImpellerType {
			CENTRIFUGAL_FORWARD_CURVED,
			CENTRIFUGAL_BACKWARD_CURVED,
			CENTRIFUGAL_AIR_FOIL,
			AXIAL,
			PROPELLER,
			UNKNOWN
		};
		const static std::unordered_map<ImpellerType, enum_info> ImpellerType_info {
			{ImpellerType::CENTRIFUGAL_FORWARD_CURVED, {"CENTRIFUGAL_FORWARD_CURVED", "Centrifugal Forward Curved", "Forward curved fan impeller"}},
			{ImpellerType::CENTRIFUGAL_BACKWARD_CURVED, {"CENTRIFUGAL_BACKWARD_CURVED", "Centrifugal Backward Curved", "Backward curved or inclined fan impeller"}},
			{ImpellerType::CENTRIFUGAL_AIR_FOIL, {"CENTRIFUGAL_AIR_FOIL", "Centrifugal Air Foil", "Air foil impeller with shaped blades"}},
			{ImpellerType::AXIAL, {"AXIAL", "Axial", "Fan impeller with shaft parallel to air flow stream for high static applications"}},
			{ImpellerType::PROPELLER, {"PROPELLER", "Propeller", "Fan impeller with shaft parallel to air flow stream for low static pressure applications"}},
			{ImpellerType::UNKNOWN, {"UNKNOWN", "None","None"}}
		};
		enum class ComponentType {
			COIL,
			FURNACE,
			FILTER,
			HEAT_EXCHANGER,
			ELECTRIC_RESISTANCE_ELEMENT,
			DIRECT_EVAPORATIVE,
			OTHER,
			UNKNOWN
		};
		const static std::unordered_map<ComponentType, enum_info> ComponentType_info {
			{ComponentType::COIL, {"COIL", "Coil", "Finned coil in cross-flow arrangement"}},
			{ComponentType::FURNACE, {"FURNACE", "Furnace", "Fuel-fired heating section"}},
			{ComponentType::FILTER, {"FILTER", "Filter", "Air filters"}},
			{ComponentType::HEAT_EXCHANGER, {"HEAT_EXCHANGER", "Heat Exchanger", "Heat exchanger"}},
			{ComponentType::ELECTRIC_RESISTANCE_ELEMENT, {"ELECTRIC_RESISTANCE_ELEMENT", "Electric Resistance Element", "Electric resistance heater elements"}},
			{ComponentType::DIRECT_EVAPORATIVE, {"DIRECT_EVAPORATIVE", "Direct Evaporative", "Wetted evaporative cooling media"}},
			{ComponentType::OTHER, {"OTHER", "Other", "Additional components in air stream"}},
			{ComponentType::UNKNOWN, {"UNKNOWN", "None","None"}}
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
			rs0003_ns::ImpellerType impeller_type;
			int number_of_impellers;
			bool manufacturer_is_set;
			bool model_number_is_set;
			bool impeller_type_is_set;
			bool number_of_impellers_is_set;
			const static std::string_view manufacturer_units;
			const static std::string_view model_number_units;
			const static std::string_view impeller_type_units;
			const static std::string_view number_of_impellers_units;
			const static std::string_view manufacturer_description;
			const static std::string_view model_number_description;
			const static std::string_view impeller_type_description;
			const static std::string_view number_of_impellers_description;
			const static std::string_view manufacturer_name;
			const static std::string_view model_number_name;
			const static std::string_view impeller_type_name;
			const static std::string_view number_of_impellers_name;
		};
		class Description  {
		public:
			rs0003_ns::ProductInformation product_information;
			bool product_information_is_set;
			const static std::string_view product_information_units;
			const static std::string_view product_information_description;
			const static std::string_view product_information_name;
		};
		class AssemblyComponent  {
		public:
			rs0003_ns::ComponentType component_type;
			std::string component_description;
			ashrae205_ns::UUID component_id;
			double wet_pressure_difference;
			bool component_type_is_set;
			bool component_description_is_set;
			bool component_id_is_set;
			bool wet_pressure_difference_is_set;
			const static std::string_view component_type_units;
			const static std::string_view component_description_units;
			const static std::string_view component_id_units;
			const static std::string_view wet_pressure_difference_units;
			const static std::string_view component_type_description;
			const static std::string_view component_description_description;
			const static std::string_view component_id_description;
			const static std::string_view wet_pressure_difference_description;
			const static std::string_view component_type_name;
			const static std::string_view component_description_name;
			const static std::string_view component_id_name;
			const static std::string_view wet_pressure_difference_name;
		};
		class SystemCurve  {
		public:
			std::vector<double> standard_air_volumetric_flow_rate;
			std::vector<double> static_pressure_difference;
			bool standard_air_volumetric_flow_rate_is_set;
			bool static_pressure_difference_is_set;
			const static std::string_view standard_air_volumetric_flow_rate_units;
			const static std::string_view static_pressure_difference_units;
			const static std::string_view standard_air_volumetric_flow_rate_description;
			const static std::string_view static_pressure_difference_description;
			const static std::string_view standard_air_volumetric_flow_rate_name;
			const static std::string_view static_pressure_difference_name;
		};
		class Performance  {
		public:
			double nominal_standard_air_volumetric_flow_rate;
			bool is_enclosed;
			std::vector<rs0003_ns::AssemblyComponent> assembly_components;
			double heat_loss_fraction;
			double maximum_impeller_rotational_speed;
			double minimum_impeller_rotational_speed;
			rs0003_ns::SystemCurve stability_curve;
			ashrae205_ns::SpeedControlType operation_speed_control_type;
			rs0003_ns::InstallationSpeedControlType installation_speed_control_type;
			rs0005_ns::RS0005 motor_representation;
			rs0007_ns::RS0007 mechanical_drive_representation;
			std::unique_ptr<PerformanceMapBase> performance_map;
			bool nominal_standard_air_volumetric_flow_rate_is_set;
			bool is_enclosed_is_set;
			bool assembly_components_is_set;
			bool heat_loss_fraction_is_set;
			bool maximum_impeller_rotational_speed_is_set;
			bool minimum_impeller_rotational_speed_is_set;
			bool stability_curve_is_set;
			bool operation_speed_control_type_is_set;
			bool installation_speed_control_type_is_set;
			bool motor_representation_is_set;
			bool mechanical_drive_representation_is_set;
			bool performance_map_is_set;
			const static std::string_view nominal_standard_air_volumetric_flow_rate_units;
			const static std::string_view is_enclosed_units;
			const static std::string_view assembly_components_units;
			const static std::string_view heat_loss_fraction_units;
			const static std::string_view maximum_impeller_rotational_speed_units;
			const static std::string_view minimum_impeller_rotational_speed_units;
			const static std::string_view stability_curve_units;
			const static std::string_view operation_speed_control_type_units;
			const static std::string_view installation_speed_control_type_units;
			const static std::string_view motor_representation_units;
			const static std::string_view mechanical_drive_representation_units;
			const static std::string_view performance_map_units;
			const static std::string_view nominal_standard_air_volumetric_flow_rate_description;
			const static std::string_view is_enclosed_description;
			const static std::string_view assembly_components_description;
			const static std::string_view heat_loss_fraction_description;
			const static std::string_view maximum_impeller_rotational_speed_description;
			const static std::string_view minimum_impeller_rotational_speed_description;
			const static std::string_view stability_curve_description;
			const static std::string_view operation_speed_control_type_description;
			const static std::string_view installation_speed_control_type_description;
			const static std::string_view motor_representation_description;
			const static std::string_view mechanical_drive_representation_description;
			const static std::string_view performance_map_description;
			const static std::string_view nominal_standard_air_volumetric_flow_rate_name;
			const static std::string_view is_enclosed_name;
			const static std::string_view assembly_components_name;
			const static std::string_view heat_loss_fraction_name;
			const static std::string_view maximum_impeller_rotational_speed_name;
			const static std::string_view minimum_impeller_rotational_speed_name;
			const static std::string_view stability_curve_name;
			const static std::string_view operation_speed_control_type_name;
			const static std::string_view installation_speed_control_type_name;
			const static std::string_view motor_representation_name;
			const static std::string_view mechanical_drive_representation_name;
			const static std::string_view performance_map_name;
		};
		class RS0003  : public RSInstanceBase {
		public:
			void initialize (const nlohmann::json& j) override;
			static std::shared_ptr<Courierr::Courierr> logger;
			ashrae205_ns::Metadata metadata;
			rs0003_ns::Description description;
			rs0003_ns::Performance performance;
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
		class GridVariablesContinuous  : public GridVariablesBase {
		public:
			void populate_performance_map (PerformanceMapBase* performance_map) override;
			enum  {
				standard_air_volumetric_flow_rate_index,
				static_pressure_difference_index,
				index_count
			};
			std::vector<double> standard_air_volumetric_flow_rate;
			std::vector<double> static_pressure_difference;
			bool standard_air_volumetric_flow_rate_is_set;
			bool static_pressure_difference_is_set;
			const static std::string_view standard_air_volumetric_flow_rate_units;
			const static std::string_view static_pressure_difference_units;
			const static std::string_view standard_air_volumetric_flow_rate_description;
			const static std::string_view static_pressure_difference_description;
			const static std::string_view standard_air_volumetric_flow_rate_name;
			const static std::string_view static_pressure_difference_name;
		};
		struct LookupVariablesContinuous  : public LookupVariablesBase {
		
			void populate_performance_map (PerformanceMapBase* performance_map) override;
			enum  {
				impeller_rotational_speed_index,
				shaft_power_index,
				index_count
			};
			std::vector<double> impeller_rotational_speed;
			std::vector<double> shaft_power;
			bool impeller_rotational_speed_is_set;
			bool shaft_power_is_set;
			const static std::string_view impeller_rotational_speed_units;
			const static std::string_view shaft_power_units;
			const static std::string_view impeller_rotational_speed_description;
			const static std::string_view shaft_power_description;
			const static std::string_view impeller_rotational_speed_name;
			const static std::string_view shaft_power_name;
		};
		struct LookupVariablesContinuousStruct {
			double impeller_rotational_speed;
			double shaft_power;
		};
		class PerformanceMapContinuous  : public PerformanceMapBase {
		public:
			void initialize (const nlohmann::json& j) override;
			rs0003_ns::GridVariablesContinuous grid_variables;
			rs0003_ns::LookupVariablesContinuous lookup_variables;
			bool grid_variables_is_set;
			bool lookup_variables_is_set;
			const static std::string_view grid_variables_units;
			const static std::string_view lookup_variables_units;
			const static std::string_view grid_variables_description;
			const static std::string_view lookup_variables_description;
			const static std::string_view grid_variables_name;
			const static std::string_view lookup_variables_name;
			using PerformanceMapBase::calculate_performance;
			LookupVariablesContinuousStruct calculate_performance (double standard_air_volumetric_flow_rate, double static_pressure_difference, Btwxt::InterpolationMethod performance_interpolation_method = Btwxt::InterpolationMethod::linear);
		};
		class GridVariablesDiscrete  : public GridVariablesBase {
		public:
			void populate_performance_map (PerformanceMapBase* performance_map) override;
			enum  {
				speed_number_index,
				static_pressure_difference_index,
				index_count
			};
			std::vector<int> speed_number;
			std::vector<double> static_pressure_difference;
			bool speed_number_is_set;
			bool static_pressure_difference_is_set;
			const static std::string_view speed_number_units;
			const static std::string_view static_pressure_difference_units;
			const static std::string_view speed_number_description;
			const static std::string_view static_pressure_difference_description;
			const static std::string_view speed_number_name;
			const static std::string_view static_pressure_difference_name;
		};
		struct LookupVariablesDiscrete  : public LookupVariablesBase {
		
			void populate_performance_map (PerformanceMapBase* performance_map) override;
			enum  {
				standard_air_volumetric_flow_rate_index,
				shaft_power_index,
				impeller_rotational_speed_index,
				index_count
			};
			std::vector<double> standard_air_volumetric_flow_rate;
			std::vector<double> shaft_power;
			std::vector<double> impeller_rotational_speed;
			bool standard_air_volumetric_flow_rate_is_set;
			bool shaft_power_is_set;
			bool impeller_rotational_speed_is_set;
			const static std::string_view standard_air_volumetric_flow_rate_units;
			const static std::string_view shaft_power_units;
			const static std::string_view impeller_rotational_speed_units;
			const static std::string_view standard_air_volumetric_flow_rate_description;
			const static std::string_view shaft_power_description;
			const static std::string_view impeller_rotational_speed_description;
			const static std::string_view standard_air_volumetric_flow_rate_name;
			const static std::string_view shaft_power_name;
			const static std::string_view impeller_rotational_speed_name;
		};
		struct LookupVariablesDiscreteStruct {
			double standard_air_volumetric_flow_rate;
			double shaft_power;
			double impeller_rotational_speed;
		};
		class PerformanceMapDiscrete  : public PerformanceMapBase {
		public:
			void initialize (const nlohmann::json& j) override;
			rs0003_ns::GridVariablesDiscrete grid_variables;
			rs0003_ns::LookupVariablesDiscrete lookup_variables;
			bool grid_variables_is_set;
			bool lookup_variables_is_set;
			const static std::string_view grid_variables_units;
			const static std::string_view lookup_variables_units;
			const static std::string_view grid_variables_description;
			const static std::string_view lookup_variables_description;
			const static std::string_view grid_variables_name;
			const static std::string_view lookup_variables_name;
			using PerformanceMapBase::calculate_performance;
			LookupVariablesDiscreteStruct calculate_performance (double speed_number, double static_pressure_difference, Btwxt::InterpolationMethod performance_interpolation_method = Btwxt::InterpolationMethod::linear);
		};
		NLOHMANN_JSON_SERIALIZE_ENUM (InstallationSpeedControlType, {
			{InstallationSpeedControlType::UNKNOWN, "UNKNOWN"},
			{InstallationSpeedControlType::FIXED, "FIXED"},
			{InstallationSpeedControlType::VARIABLE, "VARIABLE"},
		})
		NLOHMANN_JSON_SERIALIZE_ENUM (ImpellerType, {
			{ImpellerType::UNKNOWN, "UNKNOWN"},
			{ImpellerType::CENTRIFUGAL_FORWARD_CURVED, "CENTRIFUGAL_FORWARD_CURVED"},
			{ImpellerType::CENTRIFUGAL_BACKWARD_CURVED, "CENTRIFUGAL_BACKWARD_CURVED"},
			{ImpellerType::CENTRIFUGAL_AIR_FOIL, "CENTRIFUGAL_AIR_FOIL"},
			{ImpellerType::AXIAL, "AXIAL"},
			{ImpellerType::PROPELLER, "PROPELLER"},
		})
		NLOHMANN_JSON_SERIALIZE_ENUM (ComponentType, {
			{ComponentType::UNKNOWN, "UNKNOWN"},
			{ComponentType::COIL, "COIL"},
			{ComponentType::FURNACE, "FURNACE"},
			{ComponentType::FILTER, "FILTER"},
			{ComponentType::HEAT_EXCHANGER, "HEAT_EXCHANGER"},
			{ComponentType::ELECTRIC_RESISTANCE_ELEMENT, "ELECTRIC_RESISTANCE_ELEMENT"},
			{ComponentType::DIRECT_EVAPORATIVE, "DIRECT_EVAPORATIVE"},
			{ComponentType::OTHER, "OTHER"},
		})
		void from_json (const nlohmann::json& j, RS0003& x);
		void from_json (const nlohmann::json& j, Description& x);
		void from_json (const nlohmann::json& j, ProductInformation& x);
		void from_json (const nlohmann::json& j, Performance& x);
		void from_json (const nlohmann::json& j, AssemblyComponent& x);
		void from_json (const nlohmann::json& j, SystemCurve& x);
		void from_json (const nlohmann::json& j, PerformanceMapContinuous& x);
		void from_json (const nlohmann::json& j, GridVariablesContinuous& x);
		void from_json (const nlohmann::json& j, LookupVariablesContinuous& x);
		void from_json (const nlohmann::json& j, PerformanceMapDiscrete& x);
		void from_json (const nlohmann::json& j, GridVariablesDiscrete& x);
		void from_json (const nlohmann::json& j, LookupVariablesDiscrete& x);
	}
}
#endif

