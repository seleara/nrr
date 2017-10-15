#pragma once

#include <GL/glew.h>

#include <nrr/resource/shader/shader.h>

class GLShaderResource : public ShaderResource {
public:
	void bind() override;
	void release() override;
	void load(const std::string &filename) override;
private:
	bool compileShader(GLuint shader, const std::string &source, const std::string &guard);
	bool errorCheckShader(GLuint shader);
	bool errorCheckProgram(GLuint program);
	GLuint program_;
};