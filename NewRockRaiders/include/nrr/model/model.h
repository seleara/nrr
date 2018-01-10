#pragma once

#include <map>
#include <memory>
#include <string>

#include <glm/glm.hpp>

#include <nrr/resource/resource.h>
#include <nrr/resource/resourceloader.h>
#include <nrr/math/vertexbuffer.h>
#include <nrr/math/transform.h>

class ModelResource;

class ModelMesh : public Resource {
public:
	//ModelMesh(ModelResource *model) : model_(model) {}
	virtual void load(WadArchive &archive, const std::string &path) {}
	virtual void load(unsigned char *data, size_t size) {}
	virtual void fixedUpdate() {};
	virtual void update() {};
	virtual void render() = 0;
	virtual const std::string &name() const = 0;
protected:
	friend class ModelResource;
	VertexBuffer<float> buffer_;
	VertexBuffer<int> indexBuffer_;
	GLuint vao_;
	//ModelResource *model_;
};

class ModelAnimation : public Resource {
public:
	//ModelAnimation(ModelResource *model) : model_(model) {}
	virtual void load(WadArchive &archive, const std::string &path) {}
	virtual void load(unsigned char *data, size_t size) {}
	virtual void fixedUpdate() {}
	virtual void update() {}
	virtual void render() {}
	virtual const std::string &name() const = 0;
protected:
	//ModelResource *model_;

	friend class AnimationWrapper;
};

class ObjectInfo;
class LightwaveAnimation;

class AnimationWrapper {
public:
	explicit AnimationWrapper(ModelAnimation *anim);
	AnimationWrapper();
	AnimationWrapper(const AnimationWrapper &other);
	AnimationWrapper(AnimationWrapper &&other);
	AnimationWrapper &operator=(const AnimationWrapper &other);
	void wrap(ModelAnimation *anim);
	void fixedUpdate();
	void update();
	void render();
private:
	friend class ModelRenderingSystem;

	glm::mat4 calculateMatrix(LightwaveAnimation *anim, ObjectInfo &obj, int frame);

	int currentSequenceFrame_ = 0;
	int currentFrame_ = 0;
	double time_ = 0;
	struct DirtyMatrix {
		bool valid = false;
		glm::mat4 matrix;
	};
	std::vector<DirtyMatrix> matrices_;
	ModelAnimation *animation_ = nullptr;
};

class ModelResource : public Resource {
public:
	ModelMesh &mesh(const std::string &meshName) {
		return *meshes_[meshName].get();
	}
	AnimationWrapper animation(const std::string &animationName) {
		auto iter = animations_.find(animationName);
		if (iter == animations_.end()) {
			return AnimationWrapper(loadAnimation(animationName));
		}
		AnimationWrapper anim(iter->second.get());
		return anim;
	}
	AnimationWrapper externalAnimation(const std::string &path) {
		auto iter = animations_.find(path);
		if (iter == animations_.end()) {
			return AnimationWrapper(loadExternalAnimation(path));
		}
		AnimationWrapper anim(iter->second.get());
		return anim;
	}

	void fixedUpdate();
	void update();
	void render();

	virtual ModelAnimation *loadAnimation(const std::string &animationName) = 0;
	virtual ModelAnimation *loadExternalAnimation(const std::string &path) = 0;
	virtual const std::string &name() const = 0;
protected:
	friend class ModelMesh;
	friend class ModelAnimation;
	friend class ModelWrapper;

	Transform transform_;
	std::map<std::string, std::shared_ptr<ModelMesh>> meshes_;
	std::map<std::string, std::shared_ptr<ModelAnimation>> animations_;
};

class ModelWrapper {
public:
	void create(WadArchive &archive, const std::string &name);
	void load(WadArchive &archive, const std::string &path);
	void fixedUpdate();
	void update();
	void render();
	void play(const std::string &animationName);
	void playExternal(const std::string &path);
	const Transform &transform() const;
protected:
	friend class ModelRenderingSystem;

	AnimationWrapper animation_;
	std::shared_ptr<ModelResource> resource_;
};

typedef ModelWrapper Model;

#include <nrr/model/formats/lightwave.h>