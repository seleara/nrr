#pragma once

#include <string>

#include <nrr/model/model.h>
#include <nrr/resource/texture/texture.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

enum class LightwaveTextureType {
	Planar
};

enum class LightwaveTextureAnimation {
	None,
	Sequence
};

struct LightwaveTexture {
	LightwaveTextureType type;
	LightwaveTextureAnimation animation;
	Texture texture;
	uint16_t flag;
	glm::vec3 size;
	glm::vec3 center;
};

struct LightwaveSurface {
	std::string name;
	glm::vec4 color;
	uint16_t flags = 0;
	float lumi = 0.0f, diff = 0.0f, spec = 0.0f, refl = 0.0f, tran = 0.0f;
	float glos = 0.0f;
	uint16_t rflt = 0;

	LightwaveTexture ctex;
};

struct Vertex {
	glm::vec3 point;
	glm::vec2 uv;
};

class LightwaveMesh : public ModelMesh {
public:
	LightwaveMesh(ModelResource *model) : ModelMesh(model) {}
	void load(WadArchive &archive, const std::string &path) override;
	void render() override;
private:
	friend class ModelResource;
	friend class AnimatedEntityResource;
	std::vector<Vertex> points;
	std::vector<std::string> surfNames;
	std::vector<glm::ivec3> faces;
	std::vector<std::vector<glm::ivec3>> polygons;
	std::vector<int> faceSurfIndex;
	//std::vector<int> faceSurfs;
	std::vector<LightwaveSurface> surfs;
};

struct LightwaveKeyframe {
	glm::vec3 position;
	glm::quat rotation; // heading, pitch, bank
	glm::vec3 scale;
	int frame;
	enum LinearValue : int {
		Curved = 0,
		Linear = 1
	} linearValue;
	glm::vec3 splineAdjustments; // tension, continuity, bias (-1.0 ~ 1.0)
};

struct ObjectInfo {
	std::string name;
	ModelMesh *mesh = nullptr; // If mesh is null, this is a NullObject
	int parentIndex = 0;
	ObjectInfo *parent = nullptr;
	std::vector<LightwaveKeyframe> keyframes;
};

class LightwaveAnimation : public ModelAnimation {
public:
	LightwaveAnimation(ModelResource *model) : ModelAnimation(model) {}
	void load(WadArchive &archive, const std::string &path) override;
	void fixedUpdate() override;
	void render() override;
private:
	double fps_ = 0;
	int firstFrame_ = 0;
	int lastFrame_ = 0;
	int currentFrame_ = 0;
	double time_ = 0;
	glm::mat4 calculateMatrix(ObjectInfo &obj, int frame);
	LightwaveKeyframe interpolateFrames(const LightwaveKeyframe &kf1, const LightwaveKeyframe &kf2, int frame) const;
	std::vector<ObjectInfo> objects_;
};

class AnimatedEntityResource : public ModelResource {
public:
	void load(WadArchive &archive, const std::string &path) override;
private:
	friend class LightwaveMesh;
	friend class LightwaveAnimation;
};

typedef ResourceLoader<ModelResource, AnimatedEntityResource> LightwaveModelLoader;
typedef ModelWrapper Model;
