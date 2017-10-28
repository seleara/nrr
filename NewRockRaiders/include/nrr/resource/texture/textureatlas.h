#pragma once

#include <glm/glm.hpp>

#include <nrr/resource/texture/texture.h>
#include <nrr/resource/resourceloader.h>

class TextureAtlasResource : public TextureResource {
public:
	virtual void create(int width, int height) = 0;
	virtual void add(WadArchive &archive, const std::string &path, const glm::ivec2 &pixelPosition) = 0;
private:
};

class TextureAtlasWrapper {
public:
private:
	std::shared_ptr<TextureAtlasResource> resource_;
};