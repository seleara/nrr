#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <glm/glm.hpp>

#include <nrr/resource/resource.h>
#include <nrr/resource/resourceloader.h>

enum class UniformType {
	None,
	Int,
	Vec2,
	Vec3,
	Vec4
};

struct UniformInfo {
	UniformType type = UniformType::None;
	int intValue;
	glm::vec4 vecValue;
};

class ShaderResource : public Resource {
public:
	virtual void bind() = 0;
	virtual void release() = 0;
	virtual int uniformLocation(const std::string &name) const = 0;

	// Avoid using if possible, it's better to use explicit locations instead of looking them up.
	template <typename T>
	void setUniform(const std::string &uniform, const T &value) {
		setUniform(uniformLocation(uniform), value);
	}

	template <typename T>
	void setUniform(uint32_t position, const T &value) {
		static_assert(false); // Unimplemented base function
	}

	template <>
	void setUniform<int>(uint32_t position, const int &value) {
		if (isCached(position)) {
			if (getCached<int>(position) == value) return;
		}
		cache(position, value);
		setUniform1i(position, value);
	}

	template <>
	void setUniform<bool>(uint32_t position, const bool &value) {
		setUniform<int>(position, value);
	}

	template <>
	void setUniform<glm::vec2>(uint32_t position, const glm::vec2 &value) {
		if (isCached(position)) {
			if (getCached<glm::vec2>(position) == value) return;
		}
		cache(position, value);
		setUniform2fv(position, value);
	}

	template <>
	void setUniform<glm::vec3>(uint32_t position, const glm::vec3 &value) {
		if (isCached(position)) {
			if (getCached<glm::vec3>(position) == value) return;
		}
		cache(position, value);
		setUniform3fv(position, value);
	}

	template <>
	void setUniform<glm::vec4>(uint32_t position, const glm::vec4 &value) {
		if (isCached(position)) {
			if (getCached<glm::vec4>(position) == value) return;
		}
		cache(position, value);
		setUniform4fv(position, value);
	}
protected:
	virtual void setUniform1i(uint32_t position, int value) = 0;
	virtual void setUniform2fv(uint32_t position, const glm::vec2 &value) = 0;
	virtual void setUniform3fv(uint32_t position, const glm::vec3 &value) = 0;
	virtual void setUniform4fv(uint32_t position, const glm::vec4 &value) = 0;
	// This method is very picky with #include directives, should rewrite it and make it more robust later
	std::string preprocessShaderSource(const std::string &path);

	std::string source_;
	std::vector<UniformInfo> uniforms_;

	template <typename T>
	void cache(uint32_t position, const T &value) {
		if (uniforms_.size() >= position) {
			uniforms_.resize(position + 1);
		}
		UniformInfo &info = uniforms_[position];
		if constexpr (std::is_same<T, int>::value) {
			info.type = UniformType::Int;
			info.intValue = value;
		} else if constexpr (std::is_same<T, glm::vec2>::value) {
			info.type = UniformType::Vec2;
			info.vecValue = glm::vec4(value, 0, 0);
		} else if constexpr (std::is_same<T, glm::vec3>::value) {
			info.type = UniformType::Vec3;
			info.vecValue = glm::vec4(value, 0);
		} else if constexpr (std::is_same<T, glm::vec4>::value) {
			info.type = UniformType::Vec4;
			info.vecValue = value;
		} else {
			throw std::runtime_error("Unsupported uniform type.");
		}
	}

	bool isCached(uint32_t position) {
		if (position >= uniforms_.size() || uniforms_[position].type == UniformType::None) return false;
		return true;
	}

	template <typename T>
	T getCached(uint32_t position) {
		const auto &info = uniforms_[position];
		if constexpr (std::is_same<T, int>::value) {
			return info.intValue;
		} else if constexpr (std::is_same<T, glm::vec2>::value) {
			return glm::vec2(info.vecValue);
		} else if constexpr (std::is_same<T, glm::vec3>::value) {
			return glm::vec3(info.vecValue);
		} else if constexpr (std::is_same<T, glm::vec4>::value) {
			return info.vecValue;
		}
		throw std::runtime_error("Unsupported uniform type.");
	}
private:
	friend class OpenGLViewer;
};

class ShaderWrapper {
public:
	void load(const std::string &path);
	void bind();
	void release();
	int uniformLocation(const std::string &name) const;
	
	template <typename T>
	void setUniform(uint32_t position, const T &value) {
		resource_->setUniform(position, value);
	}

	void saveCache(const std::string &cacheName);
	void loadCache(const std::string &cacheName);
private:
	std::shared_ptr<ShaderResource> resource_;
};

class GLShaderResource;

typedef ResourceLoader<ShaderResource, GLShaderResource> ShaderLoader;
typedef ShaderWrapper Shader;