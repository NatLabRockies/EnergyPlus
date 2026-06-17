#include <ashrae205.h>
#include <loadobject_205.h>

namespace tk205  {

	namespace ashrae205_ns  {
	
		const std::string_view Schema::schema_title = "ASHRAE 205";

		const std::string_view Schema::schema_version = "1.0.0";

		const std::string_view Schema::schema_description = "Base schema for ASHRAE 205 representations";

		 std::shared_ptr<Courierr::Courierr> ASHRAE205::logger {};

		void from_json(const nlohmann::json& j, Metadata& x) {
			a205_json_get<std::string>(j, *ASHRAE205::logger, "data_model", x.data_model, x.data_model_is_set, true);
			a205_json_get<ashrae205_ns::SchemaType>(j, *ASHRAE205::logger, "schema", x.schema, x.schema_is_set, true);
			a205_json_get<ashrae205_ns::Version>(j, *ASHRAE205::logger, "schema_version", x.schema_version, x.schema_version_is_set, true);
			a205_json_get<ashrae205_ns::UUID>(j, *ASHRAE205::logger, "id", x.id, x.id_is_set, true);
			a205_json_get<std::string>(j, *ASHRAE205::logger, "description", x.description, x.description_is_set, true);
			a205_json_get<ashrae205_ns::Timestamp>(j, *ASHRAE205::logger, "data_timestamp", x.data_timestamp, x.data_timestamp_is_set, true);
			a205_json_get<int>(j, *ASHRAE205::logger, "data_version", x.data_version, x.data_version_is_set, true);
			a205_json_get<std::string>(j, *ASHRAE205::logger, "data_source", x.data_source, x.data_source_is_set, false);
			a205_json_get<std::string>(j, *ASHRAE205::logger, "disclaimer", x.disclaimer, x.disclaimer_is_set, false);
			a205_json_get<std::string>(j, *ASHRAE205::logger, "notes", x.notes, x.notes_is_set, false);
		}
		const std::string_view Metadata::data_model_units = "";

		const std::string_view Metadata::schema_units = "";

		const std::string_view Metadata::schema_version_units = "";

		const std::string_view Metadata::id_units = "";

		const std::string_view Metadata::description_units = "";

		const std::string_view Metadata::data_timestamp_units = "";

		const std::string_view Metadata::data_version_units = "";

		const std::string_view Metadata::data_source_units = "";

		const std::string_view Metadata::disclaimer_units = "";

		const std::string_view Metadata::notes_units = "";

		const std::string_view Metadata::data_model_description = "Data model name";

		const std::string_view Metadata::schema_description = "Schema name or identifier";

		const std::string_view Metadata::schema_version_description = "The version of the schema the data complies with";

		const std::string_view Metadata::id_description = "Unique equipment identifier";

		const std::string_view Metadata::description_description = "Description of data (suitable for display)";

		const std::string_view Metadata::data_timestamp_description = "Date of publication";

		const std::string_view Metadata::data_version_description = "Integer version identifier for the data in the representation";

		const std::string_view Metadata::data_source_description = "Source(s) of the data";

		const std::string_view Metadata::disclaimer_description = "Characterization of accuracy, limitations, and applicability of this data";

		const std::string_view Metadata::notes_description = "Additional Information";

		const std::string_view Metadata::data_model_name = "data_model";

		const std::string_view Metadata::schema_name = "schema";

		const std::string_view Metadata::schema_version_name = "schema_version";

		const std::string_view Metadata::id_name = "id";

		const std::string_view Metadata::description_name = "description";

		const std::string_view Metadata::data_timestamp_name = "data_timestamp";

		const std::string_view Metadata::data_version_name = "data_version";

		const std::string_view Metadata::data_source_name = "data_source";

		const std::string_view Metadata::disclaimer_name = "disclaimer";

		const std::string_view Metadata::notes_name = "notes";

		void from_json(const nlohmann::json& j, LiquidComponent& x) {
			a205_json_get<ashrae205_ns::LiquidConstituent>(j, *ASHRAE205::logger, "liquid_constituent", x.liquid_constituent, x.liquid_constituent_is_set, true);
			a205_json_get<double>(j, *ASHRAE205::logger, "concentration", x.concentration, x.concentration_is_set, false);
		}
		const std::string_view LiquidComponent::liquid_constituent_units = "";

		const std::string_view LiquidComponent::concentration_units = "";

		const std::string_view LiquidComponent::liquid_constituent_description = "Substance of this component of the mixture";

		const std::string_view LiquidComponent::concentration_description = "Concentration of this component of the mixture";

		const std::string_view LiquidComponent::liquid_constituent_name = "liquid_constituent";

		const std::string_view LiquidComponent::concentration_name = "concentration";

		void from_json(const nlohmann::json& j, LiquidMixture& x) {
			a205_json_get<std::vector<ashrae205_ns::LiquidComponent>>(j, *ASHRAE205::logger, "liquid_components", x.liquid_components, x.liquid_components_is_set, true);
			a205_json_get<ashrae205_ns::ConcentrationType>(j, *ASHRAE205::logger, "concentration_type", x.concentration_type, x.concentration_type_is_set, true);
		}
		const std::string_view LiquidMixture::liquid_components_units = "";

		const std::string_view LiquidMixture::concentration_type_units = "";

		const std::string_view LiquidMixture::liquid_components_description = "An array of all liquid components within the liquid mixture";

		const std::string_view LiquidMixture::concentration_type_description = "Defines whether concentration is defined on a volume or mass basis";

		const std::string_view LiquidMixture::liquid_components_name = "liquid_components";

		const std::string_view LiquidMixture::concentration_type_name = "concentration_type";

	}
}

