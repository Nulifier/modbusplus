#include "mapping-registry.hpp"

MappingRegistry& MappingRegistry::instance() {
	static MappingRegistry registry;
	return registry;
}

std::shared_ptr<Mapping> MappingRegistry::getMapping(const char* mappingPath) {
	std::string path(mappingPath);
	auto it = m_mappings.find(path);
	if (it != m_mappings.end()) {
		return it->second;
	}

	auto mapping = std::make_shared<Mapping>(mappingPath);
	m_mappings[path] = mapping;
	return mapping;
}

void MappingRegistry::removeMapping(const char* mappingPath) {
	std::string path(mappingPath);
	m_mappings.erase(path);
}

void MappingRegistry::cleanup() {
	for (auto it = m_mappings.begin(); it != m_mappings.end(); ) {
		if (it->second.use_count() == 1) {
			it = m_mappings.erase(it);
		} else {
			++it;
		}
	}
}
