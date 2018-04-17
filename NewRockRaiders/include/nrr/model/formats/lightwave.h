#pragma once

#include <string>
#include <vector>

#include <nrr/model/model.h>
#include <nrr/resource/texture/texture.h>
#include <nrr/util/configparser.h>
#include <nrr/resource/shader/shader.h>

#include <nrr/model/formats/lightwavemesh.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

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
	//LightwaveAnimation(ModelResource *model) : ModelAnimation(model) {}
	void load(WadArchive &archive, const std::string &path) override;
	void fixedUpdate() override;
	void render() override;
	const std::string &name() const override;
private:
	std::string name_;
	double fps_ = 0;
	int firstFrame_ = 0;
	int lastFrame_ = 0;
	glm::mat4 calculateMatrix(ObjectInfo &obj, int frame);
	LightwaveKeyframe interpolateFrames(const LightwaveKeyframe &kf1, const LightwaveKeyframe &kf2, int frame) const;
	std::vector<ObjectInfo> objects_;

	friend class AnimationWrapper;
	friend class ModelRenderingSystem;
};

class AnimatedEntityResource : public ModelResource {
public:
	void create(WadArchive &archive, const std::string &name) override;
	void load(WadArchive &archive, const std::string &path) override;

	ModelAnimation *loadAnimation(const std::string &animationName) override;
	ModelAnimation *loadExternalAnimation(const std::string &animationName, const std::string &path) override;
	void createAnimation(const std::string &animationName) override {
		animations_.insert({ animationName, std::make_shared<LightwaveAnimation>() });
	}
	const std::string &name() const override;
private:
	WadArchive *archive_;
	ConfigParser ae_;
	std::string name_;
	std::string folder_;

	std::string wheelNull, fireNull, drillNull, depositNull;

	friend class OpenGLViewer;
	friend class LightwaveMesh;
	friend class LightwaveAnimation;
};

class AnimatedRaiderResource : public AnimatedEntityResource {
public:
	void load(WadArchive &archive, const std::string &path) override;
};

typedef ResourceLoader<ModelResource, AnimatedEntityResource> LightwaveModelLoader;
typedef ResourceLoader<ModelAnimation, LightwaveAnimation> LightwaveAnimationLoader;
typedef ResourceLoader<ModelMesh, LightwaveMesh> LightwaveMeshLoader;