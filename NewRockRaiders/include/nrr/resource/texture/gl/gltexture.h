#pragma once

#include <GL/glew.h>

#include <nrr/resource/texture/texture.h>

class GLTextureResource : public TextureResource {
public:
	void load(WadArchive &archive, const std::string &path) override;
	void create(int width, int height, unsigned char *pixels);
	void bind() override;
	void release() override;

	GLuint id() const;
private:
	friend class OpenGLViewer;

	GLenum getEnum() const;

	GLuint textureId_;
};