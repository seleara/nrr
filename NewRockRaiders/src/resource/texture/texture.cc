#include <nrr/resource/texture/texture.h>
#include <nrr/resource/texture/gl/gltexture.h>

#include <algorithm>

#include <nrr/stb/stb_image.h>

const glm::ivec2 &TextureResource::size() const {
	return size_;
}

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
	std::copy(pixels, pixels + (width * height * 4), pixels_.begin());

	std::string label = "Memory Texture";
	glObjectLabel(texEnum, textureId_, static_cast<GLsizei>(label.size()), label.c_str());
}

void GLTextureResource::bind() {
	glBindTexture(getEnum(), textureId_);
}

void GLTextureResource::release() {
	glBindTexture(getEnum(), 0);
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

void TextureWrapper::load(WadArchive &archive, const std::string &path) {
	resource_ = TextureLoader::load(archive, path, type_);
}

void TextureWrapper::create(int width, int height, unsigned char *pixels) {
	resource_ = std::make_shared<GLTextureResource>();
	resource_->type_ = type_;
	((GLTextureResource *)resource_.get())->create(width, height, pixels);
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