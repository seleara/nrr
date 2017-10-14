#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <nrr/resource/resource.h>
#include <nrr/resource/resourceloader.h>

class ShaderResource : public Resource {
public:
	virtual void bind() = 0;
	virtual void release() = 0;
protected:
	// This method is very picky with #include directives, should rewrite it and make it more robust later
	std::string preprocessShaderSource(const std::string &path);

	std::string source_;
private:
	friend class OpenGLViewer;
};

class ShaderWrapper {
public:
	void load(WadArchive &archive, const std::string &path);
	void bind();
	void release();
private:
	std::shared_ptr<ShaderResource> resource_;
};

class GLShaderResource;

typedef ResourceLoader<ShaderResource, GLShaderResource> ShaderLoader;
typedef ShaderWrapper Shader;