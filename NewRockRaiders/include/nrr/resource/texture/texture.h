#pragma once

#include <string>
#include <memory>

#include <glm/glm.hpp>

#include <nrr/resource/resource.h>
#include <nrr/resource/wadarchive.h>

enum class TextureType {
	Normalized,
	Rectangle
};

class TextureResource : public Resource {
public:
	virtual void add(WadArchive &archive, const std::string &path, const glm::ivec2 &pixelPosition, const glm::ivec2 &destSize=glm::ivec2(0, 0)) = 0;
	virtual void create(int width, int height, unsigned char *pixels) = 0;
	virtual void bind() = 0;
	virtual void release() = 0;
	TextureType type() const {
		return type_;
	}
	const glm::ivec2 &size() const;
	virtual glm::vec4 pixel(int x, int y) const = 0;
protected:
	friend class TextureLoader;
	friend class TextureWrapper;
	glm::ivec2 size_;
	TextureType type_ = TextureType::Normalized;
};

class TextureWrapper {
public:
	void bind() {
		resource_->bind();
	}

	void release() {
		resource_->release();
	}

	TextureWrapper &normalized() {
		if (resource_) throw std::runtime_error("Can't change type of loaded texture.");
		type_ = TextureType::Normalized;
		return *this;
	}

	TextureWrapper &rectangle() {
		if (resource_) throw std::runtime_error("Can't change type of loaded texture.");
		type_ = TextureType::Rectangle;
		return *this;
	}

	TextureResource *get() const {
		return resource_.get();
	}

	bool valid() const;

	void load(WadArchive &archive, const std::string &path);

	void create(int width, int height, unsigned char *pixels);

	void add(WadArchive &archive, const std::string &path, const glm::ivec2 &pixelPosition, const glm::ivec2 &destSize = glm::ivec2(0, 0));

	void saveCache(const std::string &cacheName);
	void loadCache(const std::string &cacheName);

	const glm::ivec2 &size() const {
		return resource_->size();
	}

	glm::vec4 pixel(int x, int y) const {
		return resource_->pixel(x, y);
	}
private:
	std::shared_ptr<TextureResource> resource_;
	TextureType type_ = TextureType::Normalized;
};

class TextureLoader {
public:
	static std::shared_ptr<TextureResource> load(WadArchive &archive, std::string path, TextureType type = TextureType::Normalized);
	static std::shared_ptr<TextureResource> create(int width, int height, unsigned char *pixels, TextureType type = TextureType::Normalized);
	static void unload(const std::string &path);

	static std::shared_ptr<TextureResource> loadCache(const std::string &cacheName);
	static void saveCache(const std::string &cacheName, std::shared_ptr<TextureResource> resource);
private:
	static int createdCount_;
	friend class OpenGLViewer;
	static std::unordered_map<std::string, std::shared_ptr<TextureResource>> textures_;
	static std::unordered_map<std::string, std::shared_ptr<TextureResource>> cache_;
};

typedef TextureWrapper Texture;