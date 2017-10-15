#pragma once

#include <cstdint>
#include <iostream>
#include <map>
#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>

class GLUniformBuffer {
public:
	static void createUniformBuffer(const std::string &name, uint32_t bindingPoint, void *data, size_t size) {
		std::cout << "First: " << *((static_cast<float *>(data)) + 0) << std::endl;
		std::cout << "UBO size: " << size << std::endl;
		blocks_[name] = UniformBlockInfo{ bindingPoint, 0, size, nullptr };
		auto &block = blocks_[name];
		block.data = data;
		std::cout << "First: " << *((static_cast<float *>(data)) + 0) << std::endl;
		std::cout << "First: " << *((static_cast<float *>(block.data)) + 0) << std::endl;
		glCreateBuffers(1, &(block.buffer));
		glNamedBufferData(block.buffer, size, 0, GL_DYNAMIC_DRAW);
		for (int i = 0; i < 3; ++i) {
			for (int j = 0; j < 4; ++j) {
				for (int k = 0; k < 4; ++k) {
					std::cout << *(static_cast<float *>(block.data) + (i * 16) + j * 4 + k) << " ";
				}
				std::cout << "\n";
			}
			std::cout << "\n";
		}
	}

	static void *uniformBuffer(const std::string &name) {
		auto iter = blocks_.find(name);
		if (iter == blocks_.end()) return nullptr;
		return iter->second.data;
	}

	static void updateUniformBuffer(const std::string &name) {
		auto iter = blocks_.find(name);
		if (iter == blocks_.end()) return;
		auto &block = iter->second;
		glNamedBufferSubData(block.buffer, 0, block.size, block.data);
	}

	static void bindUniformBuffer(const std::string &name) {
		auto iter = blocks_.find(name);
		if (iter == blocks_.end()) return;
		auto &block = iter->second;
		glBindBufferRange(GL_UNIFORM_BUFFER, block.bindingPoint, block.buffer, 0, block.size);
	}
private:
	struct UniformBlockInfo {
		uint32_t bindingPoint;
		GLuint buffer;
		size_t size;
		void *data;

		~UniformBlockInfo() {
			if (data)
				delete[] data;
		}

		friend GLUniformBuffer;
	};
	static std::map<std::string, UniformBlockInfo> blocks_;
};

class UniformBuffer {
public:
	template <typename U>
	static void createUniformBuffer(const std::string &name, uint32_t bindingPoint) {
		std::cout << "Creating buffer data... ";
		U *data = new U();
		std::cout << "Done." << std::endl;
		createUniformBufferHelper(name, bindingPoint, data, sizeof(U));
	}

	template <typename U>
	static U &uniformBuffer(const std::string &name) {
		return *reinterpret_cast<U *>(uniformBufferHelper(name));
	}

	static void updateUniformBuffer(const std::string &name) {
		updateUniformBufferHelper(name);
	}

	static void bindUniformBuffer(const std::string &name) {
		bindUniformBufferHelper(name);
	}
private:
	static void createUniformBufferHelper(const std::string &name, uint32_t bindingPoint, void *data, size_t size) {
		//if (Config::graphicsAPI() == GraphicsAPI::OpenGL) {
		std::cout << "First: " << *((static_cast<float *>(data)) + 0) << std::endl;
		GLUniformBuffer::createUniformBuffer(name, bindingPoint, data, size);
		//} else if (Config::graphicsAPI() == GraphicsAPI::Vulkan) {
		// TODO...
		//}
	}

	static void *uniformBufferHelper(const std::string &name) {
		//if (Config::graphicsAPI() == GraphicsAPI::OpenGL)
		return GLUniformBuffer::uniformBuffer(name);
		//else if (Config::graphicsAPI() == GraphicsAPI::Vulkan) {
		// TODO...
		//	return nullptr;
		//}
	}

	static void updateUniformBufferHelper(const std::string &name) {
		//if (Config::graphicsAPI() == GraphicsAPI::OpenGL)
		GLUniformBuffer::updateUniformBuffer(name);
		//else if (Config::graphicsAPI() == GraphicsAPI::Vulkan) {
		// TODO...
		//}
	}

	static void bindUniformBufferHelper(const std::string &name) {
		//if (Config::graphicsAPI() == GraphicsAPI::OpenGL)
		GLUniformBuffer::bindUniformBuffer(name);
		//else if (Config::graphicsAPI() == GraphicsAPI::Vulkan) {
		// TODO...
		//}
	}
};

struct Matrices {
	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 model;
};