#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include "mapping.hpp"

class MappingRegistry {
public:
	static MappingRegistry& instance();

	std::shared_ptr<Mapping> getMapping(const char* mappingPath);
	void removeMapping(const char* mappingPath);

	/**
	 * Remove mappings that only have weak references.
	 */
	void cleanup();

private:
	MappingRegistry() = default;
	MappingRegistry(const MappingRegistry&) = delete;
	MappingRegistry& operator=(const MappingRegistry&) = delete;
	MappingRegistry& operator=(MappingRegistry&&) = delete;

	std::unordered_map<std::string, std::shared_ptr<Mapping>> m_mappings;
};
