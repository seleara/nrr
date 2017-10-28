#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>

#include <nrr/resource/resource.h>

template <typename BaseType, typename GLType, typename VkType=GLType, typename DxType=GLType>
class ResourceLoader {
public:
	static std::shared_ptr<BaseType> load(WadArchive &archive, std::string path) {
		std::transform(path.begin(), path.end(), path.begin(), ::tolower);
		auto it = resources_.find(path);
		if (it == resources_.end()) {
			std::shared_ptr<GLType> resource = std::make_shared<GLType>();
			resource->load(archive, path);
			resources_.insert({ path, resource });
			return resource;
		}
		return it->second;
	}
	static std::shared_ptr<BaseType> load(std::string path) {
		std::transform(path.begin(), path.end(), path.begin(), ::tolower);
		auto it = resources_.find(path);
		if (it == resources_.end()) {
			std::shared_ptr<GLType> resource = std::make_shared<GLType>();
			resource->load(path);
			resources_.insert({ path, resource });
			return resource;
		}
		return it->second;
	}
	static void unload(const std::string &path) {

	}

	static std::shared_ptr<BaseType> loadCache(const std::string &cacheName) {
		auto iter = cache_.find(cacheName);
		if (iter == cache_.end()) {
			return nullptr;
		}
		return iter->second;
	}

	static void saveCache(const std::string &cacheName, std::shared_ptr<BaseType> resource) {
		cache_[cacheName] = resource;
	}
private:
	friend class OpenGLViewer;
	static std::unordered_map<std::string, std::shared_ptr<BaseType>> resources_;
	static std::unordered_map<std::string, std::shared_ptr<BaseType>> cache_;
};

template <typename BaseType, typename GLType, typename VkType=GLType, typename DxType=GLType>
std::unordered_map<std::string, std::shared_ptr<BaseType>> ResourceLoader<BaseType, GLType, VkType, DxType>::resources_;

template <typename BaseType, typename GLType, typename VkType = GLType, typename DxType = GLType>
std::unordered_map<std::string, std::shared_ptr<BaseType>> ResourceLoader<BaseType, GLType, VkType, DxType>::cache_;