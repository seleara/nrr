#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>

#include <glm/glm.hpp>

#include <nrr/util/string.h>

//#define NRR_CONFIGPARSER_DEBUG

enum class ConfigNodeType {
	Value,
	Block
};

struct ConfigNode {
	std::string id;
	struct ConfigBlock *parent = nullptr;
	ConfigNodeType type;
};

struct ConfigBlock : public ConfigNode {
	std::map<std::string, std::unique_ptr<ConfigNode>> children;
};

struct ConfigValue : public ConfigNode {
	std::string value;
};

class WadArchive;

class ConfigParser {
public:
	void parse(WadArchive &archive, const std::string &path);

	bool exists(const std::string &path) const {
		return valueExists(path);
	}

	template <typename T = const std::string &>
	T get(const std::string &path) const {
		const auto &value = getValue(path);
		return value.value;
	}

	template <typename T, bool Normalized>
	T get(const std::string &path) const {
		static_assert(false);
	}

	template <>
	bool get<bool>(const std::string &path) const {
		const auto &value = getValue(path);
		auto str = value.value;
		StringUtil::toLower(str);
		if (str == "true") {
			return true;
		} else if (str == "false") {
			return false;
		}
		throw std::runtime_error("Not a boolean: \"" + path + "\"");
	}

	template <>
	glm::vec3 get<glm::vec3, true>(const std::string &path) const {
		auto ret = getVec3(path);
		ret /= 255.0f;
		return ret;
	}

	template <>
	glm::vec3 get<glm::vec3, false>(const std::string &path) const {
		auto ret = getVec3(path);
		return ret;
	}

	template <>
	glm::vec3 get<glm::vec3>(const std::string &path) const {
		auto ret = getVec3(path);
		return ret;
	}

	template <>
	glm::ivec3 get<glm::ivec3>(const std::string &path) const {
		const auto &value = getValue(path);
		auto tokens = StringUtil::splitRef(value.value, { ':' });
		glm::ivec3 ret;
		std::stringstream ss;
		ss << tokens[0] << ' ' << tokens[1] << ' ' << tokens[2];
		ss >> ret.x >> ret.y >> ret.z;
		return ret;
	}

	template <>
	float get<float>(const std::string &path) const {
		const auto &value = getValue(path);
		std::stringstream ss;
		float ret;
		ss << value.value;
		ss >> ret;
		return ret;
	}

	template <>
	float get<float, false>(const std::string &path) const {
		const auto &value = getValue(path);
		std::stringstream ss;
		float ret;
		ss << value.value;
		ss >> ret;
		ret /= 255.0f;
		return ret;
	}
private:
	glm::vec3 getVec3(const std::string &path) const {
		const auto &value = getValue(path);
		auto tokens = StringUtil::splitRef(value.value, { ':' });
		glm::vec3 ret;
		std::stringstream ss;
		ss << tokens[0] << ' ' << tokens[1] << ' ' << tokens[2];
		ss >> ret.x >> ret.y >> ret.z;
		return ret;
	}

	bool valueExists(const std::string &path) const {
		std::string remaining = path;

		// The paths used internally by the original rockraiders use "::" to separate blocks, so we need to replace all
		// occurrences of "::" with "/".
		auto doubleColon = remaining.find("::");
		while (doubleColon != std::string::npos) {
			remaining.replace(doubleColon, 2, "/");
			doubleColon = remaining.find("::");
		}
		ConfigNode *current = root_.get();
		while (remaining.size() > 0) {
			auto slashPos = remaining.find('/');
			auto next = remaining.substr(0, slashPos);
			if (slashPos == std::string::npos) remaining = "";
			else remaining.erase(0, slashPos + 1);
			//std::cout << "Getting " << next << "..." << std::endl;
			auto iter = ((ConfigBlock *)current)->children.find(next);
			if (iter == ((ConfigBlock *)current)->children.cend()) {
				return false;
			}
			current = iter->second.get();
		}
		if (current->type == ConfigNodeType::Value) {
			return true;
		}
		return false;
	}

	const ConfigValue &getValue(const std::string &path) const {
		std::string remaining = path;

		// The paths used internally by the original rockraiders use "::" to separate blocks, so we need to replace all
		// occurrences of "::" with "/".
		auto doubleColon = remaining.find("::");
		while (doubleColon != std::string::npos) {
			remaining.replace(doubleColon, 2, "/");
			doubleColon = remaining.find("::");
		}
		ConfigNode *current = root_.get();
		while (remaining.size() > 0) {
			auto slashPos = remaining.find('/');
			auto next = remaining.substr(0, slashPos);
			if (slashPos == std::string::npos) remaining = "";
			else remaining.erase(0, slashPos + 1);
			//std::cout << "Getting " << next << "..." << std::endl;
			auto iter = ((ConfigBlock *)current)->children.find(next);
			if (iter == ((ConfigBlock *)current)->children.cend()) {
				throw std::runtime_error("Couldn't find a value with the path \"" + path + "\".");
			}
			current = iter->second.get();
		}
		if (current->type == ConfigNodeType::Value) {
			return *((ConfigValue *)current);
		}
		throw std::runtime_error("Couldn't find a value with the path \"" + path + "\".");
	}

	std::unique_ptr<ConfigBlock> root_;
};