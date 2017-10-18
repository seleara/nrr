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
		auto tokens = StringUtil::split(value.value, { ':' });
		glm::ivec3 ret;
		std::stringstream ss;
		ss << tokens[0] << ' ' << tokens[1] << ' ' << tokens[2];
		ss >> ret.x >> ret.y >> ret.z;
		return ret;
	}
private:
	glm::vec3 getVec3(const std::string &path) const {
		const auto &value = getValue(path);
		auto tokens = StringUtil::split(value.value, { ':' });
		glm::vec3 ret;
		std::stringstream ss;
		ss << tokens[0] << ' ' << tokens[1] << ' ' << tokens[2];
		ss >> ret.x >> ret.y >> ret.z;
		return ret;
	}

	const ConfigValue &getValue(const std::string &path) const {
		std::string remaining = path;
		ConfigNode *current = root_.get();
		while (remaining.size() > 0) {
			auto slashPos = remaining.find('/');
			auto next = remaining.substr(0, slashPos);
			if (slashPos == std::string::npos) remaining = "";
			else remaining.erase(0, slashPos + 1);
			//std::cout << "Getting " << next << "..." << std::endl;
			current = ((ConfigBlock *)current)->children[next].get();
		}
		if (current->type == ConfigNodeType::Value) {
			return *((ConfigValue *)current);
		}
		throw std::runtime_error("Couldn't find a value with the path \"" + path + "\".");
	}

	std::unique_ptr<ConfigBlock> root_;
};