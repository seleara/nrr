#include <nrr/resource/shader/gl/glshader.h>

#include <fstream>
#include <iostream>

void GLShaderResource::load(const std::string &path) {
	source_ = preprocessShaderSource(path);

	auto vs = glCreateShader(GL_VERTEX_SHADER);
	auto fs = glCreateShader(GL_FRAGMENT_SHADER);

	if (!compileShader(vs, source_, "NRR_SHADER_VERTEX") || !compileShader(fs, source_, "NRR_SHADER_FRAGMENT")) {
		throw std::runtime_error("Unable to compile shader.");
	}

	program_ = glCreateProgram();
	glAttachShader(program_, vs);
	glAttachShader(program_, fs);
	glLinkProgram(program_);
	if (!errorCheckProgram(program_)) {
		glDetachShader(program_, vs);
		glDetachShader(program_, fs);
		glDeleteShader(vs);
		glDeleteShader(fs);
		glDeleteProgram(program_);
		throw std::runtime_error("Unable to link program.");
	}
	glDetachShader(program_, vs);
	glDetachShader(program_, fs);
	glDeleteShader(vs);
	glDeleteShader(fs);
}

int GLShaderResource::uniformLocation(const std::string &name) const {
	return glGetUniformLocation(program_, name.c_str());
}

bool GLShaderResource::compileShader(GLuint shader, const std::string &source, const std::string &guard) {
	std::string processed = "#version 450\n#define " + guard + "\n" + source;
	const char *csource = processed.c_str();
	glShaderSource(shader, 1, &csource, nullptr);
	glCompileShader(shader);
	if (!errorCheckShader(shader)) {
		glDeleteShader(shader);
		return false;
	}
	return true;
}

bool GLShaderResource::errorCheckShader(GLuint shader) {
	GLint isCompiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE) {
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

		std::cerr << &errorLog[0] << std::endl;

		return false;
	}
	return true;
}

bool GLShaderResource::errorCheckProgram(GLuint program) {
	GLint isLinked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
	if (isLinked == GL_FALSE) {
		GLint maxLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
		std::vector<GLchar> errorLog(maxLength);
		glGetProgramInfoLog(program, maxLength, &maxLength, &errorLog[0]);

		std::cerr << &errorLog[0] << std::endl;

		return false;
	}
	return true;
}

void GLShaderResource::bind() {
	glUseProgram(program_);
}

void GLShaderResource::release() {
	glUseProgram(0);
}