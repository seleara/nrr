#pragma once

#include <GL/glew.h>

#include <nrr/resource/texture/texture.h>

class GLTextureResource : public TextureResource {
public:
	void load(WadArchive &archive, const std::string &path) override;
	void create(int width, int height, unsigned char *pixels) override;
	void add(WadArchive &archive, const std::string &path, const glm::ivec2 &pixelPosition, const glm::ivec2 &destSize = glm::ivec2(0, 0)) override;
	void bind() override;
	void release() override;
	glm::vec4 pixel(int x, int y) const override;

	GLuint id() const;
private:
	friend class OpenGLViewer;

	GLenum getEnum() const;

	std::vector<unsigned char> pixels_;

	GLuint textureId_;
};