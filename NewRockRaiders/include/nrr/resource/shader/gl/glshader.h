#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <nrr/resource/shader/shader.h>

class GLShaderResource : public ShaderResource {
public:
	void bind() override;
	void release() override;
	void load(const std::string &filename) override;
	int uniformLocation(const std::string &name) const override;
protected:
	void setUniform1i(uint32_t position, int value) override;
	void setUniform2fv(uint32_t position, const glm::vec2 &value) override;
	void setUniform3fv(uint32_t position, const glm::vec3 &value) override;
	void setUniform4fv(uint32_t position, const glm::vec4 &value) override;
private:
	bool compileShader(GLuint shader, const std::string &source, const std::string &guard);
	bool errorCheckShader(GLuint shader);
	bool errorCheckProgram(GLuint program);
	GLuint program_;
};