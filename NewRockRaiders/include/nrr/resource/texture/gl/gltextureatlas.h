#pragma once

#include <GL/glew.h>

#include <nrr/resource/texture/textureatlas.h>

class GLTextureAtlasResource : public TextureAtlasResource {
public:
	void create(int width, int height) override;
	void add(WadArchive &archive, const std::string &path, const glm::ivec2 &pixelPosition) override;
	void bind() override;
	void release() override;
private:
	GLuint texture_;
};