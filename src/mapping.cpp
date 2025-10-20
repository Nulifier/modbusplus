#include "mapping.hpp"
#include <fstream>
#include "nlohmann/json.hpp"

Mapping::Mapping(const char* path) {
	// Read and parse the mapping file located at 'path'
	std::ifstream file(path);
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open mapping file");
	}

	// Read JSON content
	std::string content((std::istreambuf_iterator<char>(file)),
						std::istreambuf_iterator<char>());

	nlohmann::json j;
	try {
		j = nlohmann::json::parse(content);
	} catch (const nlohmann::json::parse_error& e) {
		throw std::runtime_error(std::string("JSON parse error: ") + e.what());
	}

	// Must have a "values" object
	if (!j.contains("values") || !j["values"].is_object()) {
		throw std::runtime_error("Mapping file must contain a 'values' object");
	}

	// Read each value definition
	for (const auto& item : j["values"].items()) {
		ValueDef def;

		def.addr = item.value().at("addr").get<uint16_t>();

		auto formatStr =
			item.value().at("format").get_ref<const std::string&>();
		def.format = ValueDefFormat::bit;
		if (formatStr == "bit") {
			def.format = ValueDefFormat::bit;
		} else if (formatStr == "u16") {
			def.format = ValueDefFormat::u16;
		} else if (formatStr == "i16") {
			def.format = ValueDefFormat::i16;
		} else if (formatStr == "u32") {
			def.format = ValueDefFormat::u32;
		} else if (formatStr == "i32") {
			def.format = ValueDefFormat::i32;
		} else if (formatStr == "u64") {
			def.format = ValueDefFormat::u64;
		} else if (formatStr == "i64") {
			def.format = ValueDefFormat::i64;
		} else if (formatStr == "f32") {
			def.format = ValueDefFormat::f32;
		} else if (formatStr == "f64") {
			def.format = ValueDefFormat::f64;
		} else if (formatStr == "str") {
			def.format = ValueDefFormat::str;
		} else if (formatStr == "bitfield") {
			def.format = ValueDefFormat::bitfield;
		} else {
			throw std::runtime_error("Invalid format in mapping for key: " +
									 item.key());
		}

		auto typeStr = item.value().at("type").get_ptr<const std::string*>();
		def.type = ValueDefType::holding;
		if (typeStr && *typeStr == "input") {
			def.type = ValueDefType::input;
		} else {
			def.type = ValueDefType::holding;
		}

		// Order is optional; default depends on format
		def.order = ValueDefOrder::ab;
		auto orderStr =
			item.value().contains("order")
				? item.value().at("order").get_ptr<const std::string*>()
				: nullptr;
		switch (def.format) {
			case ValueDefFormat::bit:
				throw std::runtime_error(
					"Bit format does not support byte order for key: " +
					item.key());
				break;
			case ValueDefFormat::u16:
			case ValueDefFormat::i16:
				if (!orderStr || *orderStr == "ab") {
					def.order = ValueDefOrder::ab;
				} else if (*orderStr == "ba") {
					def.order = ValueDefOrder::ba;
				} else {
					throw std::runtime_error(
						"Invalid order in mapping for key: " + item.key());
				}
				break;
			case ValueDefFormat::u32:
			case ValueDefFormat::i32:
			case ValueDefFormat::f32:
				if (!orderStr || *orderStr == "abcd") {
					def.order = ValueDefOrder::abcd;
				} else if (*orderStr == "dcba") {
					def.order = ValueDefOrder::dcba;
				} else if (*orderStr == "badc") {
					def.order = ValueDefOrder::badc;
				} else if (*orderStr == "cdab") {
					def.order = ValueDefOrder::cdab;
				} else {
					throw std::runtime_error(
						"Invalid order in mapping for key: " + item.key());
				}
				break;
			case ValueDefFormat::u64:
			case ValueDefFormat::i64:
			case ValueDefFormat::f64:
				if (!orderStr || *orderStr == "abcdefgh") {
					def.order = ValueDefOrder::abcdefgh;
				} else {
					throw std::runtime_error(
						"Invalid order in mapping for key: " + item.key());
				}
				break;
			case ValueDefFormat::str:
				if (!orderStr || *orderStr == "ab") {
					def.order = ValueDefOrder::ab;
				} else if (*orderStr == "ba") {
					def.order = ValueDefOrder::ba;
				} else if (*orderStr == "a") {
					def.order = ValueDefOrder::a;
				} else if (*orderStr == "b") {
					def.order = ValueDefOrder::b;
				} else {
					throw std::runtime_error(
						"Invalid order in mapping for key: " + item.key());
				}
			default:
				break;
		}

		// Scale should only be present for numeric types
		if (def.format == ValueDefFormat::str ||
			def.format == ValueDefFormat::bit ||
			def.format == ValueDefFormat::bitfield) {
			if (item.value().contains("scale")) {
				throw std::runtime_error(
					"Scale not applicable for format in key: " + item.key());
			}
		}
		def.scale = item.value().value("scale", 1.0);

		// Length should only be present for string and bitfield types
		if (def.format != ValueDefFormat::str &&
			def.format != ValueDefFormat::bitfield) {
			if (item.value().contains("length")) {
				throw std::runtime_error(
					"Length not applicable for format in key: " + item.key());
			}
		}
		def.length = item.value().value("length", 1);
		if (def.format == ValueDefFormat::u32 ||
			def.format == ValueDefFormat::i32 ||
			def.format == ValueDefFormat::f32) {
			def.length = 2;
		} else if (def.format == ValueDefFormat::u64 ||
				   def.format == ValueDefFormat::i64 ||
				   def.format == ValueDefFormat::f64) {
			def.length = 4;
		}

		def.linked = item.value().value("linked", "");
		if ((!def.linked.empty()) && (def.format == ValueDefFormat::bit ||
									  def.format == ValueDefFormat::str)) {
			throw std::runtime_error(
				"Linking not applicable for format in key: " + item.key());
		}

		m_values[item.key()] = def;
	}
	printf("DEBUG: Loaded %d value definitions\n", (int)m_values.size());

	// Read bitfield definitions
	if (j.contains("bitfields")) {
		if (!j["bitfields"].is_object()) {
			throw std::runtime_error(
				"'bitfields' must be an object in mapping file");
		}

		for (const auto& item : j["bitfields"].items()) {
			const auto& bitfieldName = item.key();

			// Bitfield are a string key and an object with string keys and
			// integer values
			if (!item.value().is_object()) {
				throw std::runtime_error(
					"Bitfield definition must be an object for key: " +
					item.key());
			}

			for (const auto& bf_item : item.value().items()) {
				const auto& bitName = bf_item.key();

				if (!bf_item.value().is_number_integer()) {
					throw std::runtime_error(
						"Bitfield value must be an integer for key: " +
						bf_item.key());
				}

				uint16_t bitPos = bf_item.value().get<uint16_t>();
				m_bitfields[bitfieldName][bitPos] = bitName;
			}
		}
	}
	printf("DEBUG: Loaded %d bitfields\n", (int)m_bitfields.size());

	// Read enum definitions
	if (j.contains("enums")) {
		if (!j["enums"].is_object()) {
			throw std::runtime_error(
				"'enums' must be an object in mapping file");
		}

		for (const auto& item : j["enums"].items()) {
			const auto& enumName = item.key();

			if (!item.value().is_object()) {
				throw std::runtime_error(
					"Enum definition must be an object for key: " + item.key());
			}

			for (const auto& enum_item : item.value().items()) {
				if (!enum_item.value().is_number_integer()) {
					throw std::runtime_error(
						"Enum value must be an integer for key: " +
						enum_item.key());
				}

				int64_t enumValue = enum_item.value().get<int64_t>();
				m_enums[enumName][enumValue] = enum_item.key();
			}
		}
	}
	printf("DEBUG: Loaded %d enums\n", (int)m_enums.size());
}

const Mapping::ValueDef& Mapping::getValueDef(const std::string& name) const {
	auto it = m_values.find(name);
	if (it == m_values.end()) {
		throw std::runtime_error("Value definition not found for key: " + name);
	}
	return it->second;
}

const std::unordered_map<uint16_t, std::string>& Mapping::getBitfieldDef(
	const std::string& name) const {
	auto it = m_bitfields.find(name);
	if (it == m_bitfields.end()) {
		throw std::runtime_error("Bitfield definition not found for key: " +
								 name);
	}
	return it->second;
}

const std::unordered_map<int64_t, std::string>& Mapping::getEnumDef(
	const std::string& name) const {
	auto it = m_enums.find(name);
	if (it == m_enums.end()) {
		throw std::runtime_error("Enum definition not found for key: " + name);
	}
	return it->second;
}
