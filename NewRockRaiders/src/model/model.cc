#include <nrr/model/model.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <nrr/model/formats/lightwave.h>
#include <nrr/resource/shader/shader.h>
#include <nrr/math/time.h>
#include <nrr/math/uniformbuffer.h>

void ModelResource::fixedUpdate() {
	/*for (auto &kv : meshes_) {
	auto &m = *kv.second.get();
	m.render();
	}*/
	for (auto &kv : animations_) {
		auto &anim = *kv.second.get();
		anim.fixedUpdate();
	}
}

void ModelResource::update() {
	/*for (auto &kv : meshes_) {
	auto &m = *kv.second.get();
	m.render();
	}*/
	for (auto &kv : animations_) {
		auto &anim = *kv.second.get();
		anim.update();
	}
}

void ModelResource::render() {
	for (auto &kv : animations_) {
		auto &anim = *kv.second.get();
		anim.render();
	}
}

AnimationWrapper::AnimationWrapper(ModelAnimation *animation) : animation_(animation) {
	auto *anim = ((LightwaveAnimation *)animation_);
	currentFrame_ = anim->firstFrame_;
	matrices_.resize(anim->objects_.size());
}

AnimationWrapper::AnimationWrapper() : animation_(nullptr) {

}

AnimationWrapper::AnimationWrapper(const AnimationWrapper &other) : animation_(other.animation_) {
	auto *anim = ((LightwaveAnimation *)animation_);
	currentFrame_ = anim->firstFrame_;
	matrices_.resize(anim->objects_.size());
}

AnimationWrapper::AnimationWrapper(AnimationWrapper &&other) : animation_(std::move(other.animation_)) {
	auto *anim = ((LightwaveAnimation *)animation_);
	currentFrame_ = anim->firstFrame_;
	matrices_.resize(anim->objects_.size());
}

AnimationWrapper &AnimationWrapper::operator=(const AnimationWrapper &other) {
	animation_ = other.animation_;
	currentFrame_ = other.currentFrame_;
	matrices_ = other.matrices_;
	return *this;
}

void AnimationWrapper::wrap(ModelAnimation *animation) {
	animation_ = animation;
	auto *anim = ((LightwaveAnimation *)animation_);
	currentFrame_ = anim->firstFrame_;
	matrices_.resize(anim->objects_.size());
}

void AnimationWrapper::fixedUpdate() {
	if (!animation_) return;
	auto anim = (LightwaveAnimation *)animation_;
	time_ += Time::fixedDeltaTime();
	while (time_ >= (1.0 / anim->fps_)) {
		time_ -= (1.0 / anim->fps_);
		++currentFrame_;
		if (currentFrame_ > anim->lastFrame_) {
			currentFrame_ = anim->firstFrame_;
		}
	}
	/*for (auto &obj : animation_->objects_) {
		if (obj.mesh) {
			obj.mesh->fixedUpdate();
		}
	}*/
}

void AnimationWrapper::update() {
	if (!animation_) return;
	auto anim = (LightwaveAnimation *)animation_;

	// Calculate matrices for rendering
	for (auto &mat : matrices_) {
		mat.valid = false;
	}
	for (int i = 0; i < anim->objects_.size(); ++i) {
		auto &obj = anim->objects_[i];
		matrices_[i].matrix = calculateMatrix(anim, obj, currentFrame_);
		matrices_[i].valid = true;
	}
}

glm::mat4 AnimationWrapper::calculateMatrix(LightwaveAnimation *anim, ObjectInfo &obj, int frame) {
	glm::mat4 m;
	if (obj.parent) {
		if (matrices_[obj.parentIndex - 1].valid) {
			m = matrices_[obj.parentIndex - 1].matrix;
		} else {
			m = calculateMatrix(anim, *obj.parent, frame);
		}
	}
	const LightwaveKeyframe *kf1 = nullptr, *kf2 = nullptr;
	for (int i = 0; i < obj.keyframes.size(); ++i) {
		const auto &kfi = obj.keyframes[i];
		const auto &kfi1 = obj.keyframes[(i + 1) % obj.keyframes.size()];
		if (kfi.frame <= frame && kfi1.frame > frame) {
			kf1 = &kfi;
			kf2 = &kfi1;
		} else if (kfi.frame <= frame && kfi1.frame == anim->firstFrame_ - 1) {
			kf1 = &kfi;
			kf2 = &kfi1;
		}
	}
	if (kf1 == nullptr) {
		kf1 = &obj.keyframes[0];
		kf2 = kf1;
	}
	auto kf = anim->interpolateFrames(*kf1, *kf2, frame);
	/*auto rotM = glm::toMat4(kf.rotation);
	auto transM = glm::translate(glm::mat4(), kf.position);
	auto scaleM = glm::scale(glm::mat4(), kf.scale);
	m = m * transM * scaleM * rotM;*/
	m = glm::translate(m, kf.position);
	m = glm::scale(m, kf.scale);
	m *= glm::toMat4(kf.rotation);
	return m;
}

void AnimationWrapper::render() {
	if (!animation_) return;
	auto anim = (LightwaveAnimation *)animation_;
	//for (auto &obj : anim->objects_) {
	auto mvp = UniformBuffer::uniformBuffer<Matrices>("mvp");
	for (int i = 0; i < anim->objects_.size(); ++i) {
		auto &obj = anim->objects_[i];
		if (obj.mesh) {
			mvp->model = matrices_[i].matrix;
			//UniformBuffer::updateUniformBuffer("mvp");
			mvp.update();
			obj.mesh->render();
		}
	}
}

void ModelWrapper::create(WadArchive &archive, const std::string &name) {
	resource_ = LightwaveModelLoader::create(archive, name);
}

void ModelWrapper::load(WadArchive &archive, const std::string &path) {
	resource_ = LightwaveModelLoader::load(archive, path);
	play("Activity_Stand");
}

void ModelWrapper::fixedUpdate() {
	//resource_->fixedUpdate();
	animation_.fixedUpdate();
}

void ModelWrapper::update() {
	//resource_->update();
	animation_.update();
}

void ModelWrapper::render() {
	//resource_->render();
	animation_.render();
}

void ModelWrapper::play(const std::string &animationName) {
	animation_ = resource_->animation(animationName);
}

void ModelWrapper::playExternal(const std::string &path) {
	animation_ = resource_->externalAnimation(path);
}