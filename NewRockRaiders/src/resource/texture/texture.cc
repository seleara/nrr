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

	glObjectLabel(texEnum, textureId_, static_cast<GLsizei>(path.size()), path.c_str());

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

void TextureWrapper::load(WadArchive &archive, const std::string &path) {
	resource_ = TextureLoader::load(archive, path, type_);
}

void TextureWrapper::create(int width, int height, unsigned char *pixels) {
	resource_ = std::make_shared<GLTextureResource>();
	resource_->type_ = type_;
	((GLTextureResource *)resource_.get())->create(width, height, pixels);
}

bool TextureWrapper::valid() const {
	if (!resource_) return false;
	return ((GLTextureResource *)resource_.get())->id() != 0;
}

std::unordered_map<std::string, std::shared_ptr<TextureResource>> TextureLoader::textures_;

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