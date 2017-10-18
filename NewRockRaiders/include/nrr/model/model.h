#pragma once

#include <map>
#include <memory>
#include <string>

#include <nrr/resource/resource.h>
#include <nrr/resource/resourceloader.h>
#include <nrr/math/vertexbuffer.h>

class ModelResource;

class ModelMesh {
public:
	ModelMesh(ModelResource *model) : model_(model) {}
	virtual void load(WadArchive &archive, const std::string &path) {}
	virtual void load(unsigned char *data, size_t size) {}
	virtual void render() = 0;
protected:
	friend class ModelResource;
	VertexBuffer<float> buffer_;
	VertexBuffer<int> indexBuffer_;
	GLuint vao_;
	ModelResource *model_;
};

class ModelAnimation {
public:
	ModelAnimation(ModelResource *model) : model_(model) {}
	virtual void load(WadArchive &archive, const std::string &path) {}
	virtual void load(unsigned char *data, size_t size) {}
	virtual void fixedUpdate() {}
	virtual void update() {}
	virtual void render() {}
protected:
	ModelResource *model_;
};

class ModelResource : public Resource {
public:
	ModelMesh &mesh(const std::string &meshName) {
		return *meshes_[meshName].get();
	}
	ModelAnimation &animation(const std::string &animationName) {
		return *animations_[animationName].get();
	}

	void fixedUpdate();
	void update();
	void render();
protected:
	friend class ModelMesh;
	friend class ModelAnimation;
	std::map<std::string, std::unique_ptr<ModelMesh>> meshes_;
	std::map<std::string, std::unique_ptr<ModelAnimation>> animations_;
};

class ModelWrapper {
public:
	void load(WadArchive &archive, const std::string &path);
	void fixedUpdate();
	void update();
	void render();
private:
	std::shared_ptr<ModelResource> resource_;
};

#include <nrr/model/formats/lightwave.h>