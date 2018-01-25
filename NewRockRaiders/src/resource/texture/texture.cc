#include <nrr/resource/texture/texture.h>
#include <nrr/resource/texture/gl/gltexture.h>

#include <algorithm>
#include <sstream>

#include <nrr/stb/stb_image.h>

#include <iostream>

const glm::ivec2 &TextureResource::size() const {
	return size_;
}

GLuint GLTextureResource::currentTextureId_ = 0;

void GLTextureResource::load(WadArchive &archive, const std::string &path) {
	const auto &entry = archive.get(path);
	auto &is = archive.getStream(entry);
	unsigned char *buffer = new unsigned char[entry.size];
	is.read(reinterpret_cast<char *>(buffer), entry.size);
	int width, height, channels;
	unsigned char *data = stbi_load_from_memory(buffer, entry.size, &width, &height, &channels, 4);
	size_.x = width;
	size_.y = height;

	auto texEnum = getEnum();

	glGenTextures(1, &textureId_);
	bind();
	glTexParameteri(texEnum, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(texEnum, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(texEnum, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(texEnum, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(texEnum, 0, GL_RGBA, size_.x, size_.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	release();

	pixels_.resize(width * height * 4);
	std::copy(data, data + (width * height * 4), pixels_.begin());

	glObjectLabel(GL_TEXTURE, textureId_, static_cast<GLsizei>(path.size()), path.c_str());

	loadPalette(buffer);

	delete[] buffer;
	stbi_image_free(data);
}

void GLTextureResource::create(int width, int height, unsigned char *pixels) {
	size_.x = width;
	size_.y = height;
	auto texEnum = getEnum();
	glGenTextures(1, &textureId_);
	bind();
	glTexParameteri(texEnum, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(texEnum, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(texEnum, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(texEnum, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(texEnum, 0, GL_RGBA, size_.x, size_.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	release();

	pixels_.resize(width * height * 4);
	if (pixels) {
		std::copy(pixels, pixels + (width * height * 4), pixels_.begin());
	}

	std::string label = "Memory Texture";
	glObjectLabel(texEnum, textureId_, static_cast<GLsizei>(label.size()), label.c_str());
}

void GLTextureResource::add(WadArchive &archive, const std::string &path, const glm::ivec2 &pixelPosition, const glm::ivec2 &destSize) {
	const auto &entry = archive.get(path);
	auto &is = archive.getStream(entry);
	unsigned char *buffer = new unsigned char[entry.size];
	is.read(reinterpret_cast<char *>(buffer), entry.size);
	int width, height, channels;
	unsigned char *data = stbi_load_from_memory(buffer, entry.size, &width, &height, &channels, 4);

	unsigned char *pixels;
	/*if (destSize.x != 0 && destSize.y != 0) {
		// Custom size
		if ((destSize.x % width) != 0 || (destSize.y % height) != 0) {
			throw std::runtime_error("Unsupported destination size. Only multiples of the original size are supported.");
		}
		auto mulY = destSize.y / height;
		auto mulX = destSize.x / width;
		pixels = new unsigned char[width * height * mulX * mulY];
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				for (int my = 0; my < mulY; ++my) {
					for (int mx = 0; mx < mulX; ++mx) {
						pixels[y * mulY * width * mulX + ]
					}
				}
			}
		}
		width *= mulX;
		height *= mulY;
	} else {*/
		pixels = data;
	//}

	auto texEnum = getEnum();
	bind();
	glTexSubImage2D(texEnum, 0, pixelPosition.x, pixelPosition.y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	release();

	std::copy(pixels, pixels + (width * height * 4), pixels_.begin() + (pixelPosition.y * size_.x + pixelPosition.x) * 4);

	loadPalette(buffer);
	
	delete[] buffer;
	stbi_image_free(data);
}

void GLTextureResource::loadPalette(const unsigned char *buffer) {
	if (strncmp((const char *)buffer, "BM", 2) == 0) {
		auto bpp = *(uint16_t *)(buffer + 0x1c);
		if (bpp < 16) {
			auto colors = *(uint32_t *)(buffer + 0x2e);
			if (colors == 0) {
				colors = std::pow(2, bpp);
			}
			auto paletteOffset = 0x36;
			for (int i = 0; i < colors; ++i) {
				glm::ivec4 color;
				color.b = buffer[paletteOffset + 4 * i + 0];
				color.g = buffer[paletteOffset + 4 * i + 1];
				color.r = buffer[paletteOffset + 4 * i + 2];
				color.a = 255.0f;
				palette_.push_back(color);
			}
		} else {
			std::cout << std::endl;
		}
	} else {
		std::cout << std::endl;
	}
}

void GLTextureResource::bind() {
	if (textureId_ != currentTextureId_) {
		glBindTexture(getEnum(), textureId_);
		currentTextureId_ = textureId_;
	}
}

void GLTextureResource::release() {
	glBindTexture(getEnum(), 0);
	currentTextureId_ = 0;
}

GLuint GLTextureResource::id() const {
	return textureId_;
}

GLenum GLTextureResource::getEnum() const {
	return type_ == TextureType::Normalized ? GL_TEXTURE_2D : GL_TEXTURE_RECTANGLE;
}

glm::vec4 GLTextureResource::pixel(int x, int y) const {
	int i = (y * size_.x + x) * 4;
	return glm::vec4(pixels_[i + 0], pixels_[i + 1], pixels_[i + 2], pixels_[i + 3]) / 255.0f;
}

glm::vec4 GLTextureResource::palette(int index) const {
	return palette_[index];
}

void TextureWrapper::load(WadArchive &archive, const std::string &path) {
	resource_ = TextureLoader::load(archive, path, type_);
}

void TextureWrapper::create(int width, int height, unsigned char *pixels) {
	resource_ = TextureLoader::create(width, height, pixels);
}

void TextureWrapper::add(WadArchive &archive, const std::string &path, const glm::ivec2 &pixelPosition, const glm::ivec2 &destSize) {
	resource_->add(archive, path, pixelPosition, destSize);
}

void TextureWrapper::saveCache(const std::string &cacheName) {
	TextureLoader::saveCache(cacheName, resource_);
}

void TextureWrapper::loadCache(const std::string &cacheName) {
	resource_ = TextureLoader::loadCache(cacheName);
}

bool TextureWrapper::valid() const {
	if (!resource_) return false;
	return ((GLTextureResource *)resource_.get())->id() != 0;
}

std::unordered_map<std::string, std::shared_ptr<TextureResource>> TextureLoader::textures_;
std::unordered_map<std::string, std::shared_ptr<TextureResource>> TextureLoader::cache_;

std::shared_ptr<TextureResource> TextureLoader::load(WadArchive &archive, std::string path, TextureType type) {
	std::transform(path.begin(), path.end(), path.begin(), ::tolower);
	auto it = textures_.find(path);
	if (it == textures_.end()) {
		std::shared_ptr<TextureResource> resource = std::make_shared<GLTextureResource>();
		resource->type_ = type;
		resource->load(archive, path);
		textures_.insert({ path, resource });
		return resource;
	}
	return it->second;
}

int TextureLoader::createdCount_ = 0;

std::shared_ptr<TextureResource> TextureLoader::create(int width, int height, unsigned char *pixels, TextureType type) {
	std::shared_ptr<TextureResource> resource = std::make_shared<GLTextureResource>();
	resource->type_ = type;
	resource->create(width, height, pixels);
	std::stringstream ss;
	ss << "Memory Texture " << createdCount_;
	textures_.insert({ ss.str(), resource });
	return resource;
}

void TextureLoader::unload(const std::string &path) {

}


std::shared_ptr<TextureResource> TextureLoader::loadCache(const std::string &cacheName) {
	auto iter = cache_.find(cacheName);
	if (iter == cache_.end()) {
		return nullptr;
	}
	return iter->second;
}

void TextureLoader::saveCache(const std::string &cacheName, std::shared_ptr<TextureResource> resource) {
	cache_[cacheName] = resource;
}