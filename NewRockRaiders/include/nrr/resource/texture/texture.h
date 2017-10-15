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
	virtual void bind() = 0;
	virtual void release() = 0;
	const glm::ivec2 &size() const;
protected:
	friend class TextureLoader;
	friend class TextureWrapper;
	glm::ivec2 size_;
	TextureType type_ = TextureType::Normalized;
};

class TextureWrapper {
public:
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

	const glm::ivec2 &size() const {
		return resource_->size();
	}
private:
	std::shared_ptr<TextureResource> resource_;
	TextureType type_ = TextureType::Normalized;
};

class TextureLoader {
public:
	static std::shared_ptr<TextureResource> load(WadArchive &archive, std::string path, TextureType type=TextureType::Normalized);
	static void unload(const std::string &path);
private:
	friend class OpenGLViewer;
	static std::unordered_map<std::string, std::shared_ptr<TextureResource>> textures_;
};

typedef TextureWrapper Texture;