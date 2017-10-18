#include <nrr/model/model.h>

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

AnimationWrapper::AnimationWrapper(ModelAnimation *anim) : animation_(anim) {
	currentFrame_ = ((LightwaveAnimation *)animation_)->firstFrame_;
}

AnimationWrapper::AnimationWrapper() : animation_(nullptr) {

}

AnimationWrapper::AnimationWrapper(const AnimationWrapper &other) : animation_(other.animation_) {
	currentFrame_ = ((LightwaveAnimation *)animation_)->firstFrame_;
}

AnimationWrapper::AnimationWrapper(AnimationWrapper &&other) : animation_(std::move(other.animation_)) {
	currentFrame_ = ((LightwaveAnimation *)animation_)->firstFrame_;
}

AnimationWrapper &AnimationWrapper::operator=(const AnimationWrapper &other) {
	animation_ = other.animation_;
	currentFrame_ = other.currentFrame_;
	return *this;
}

void AnimationWrapper::wrap(ModelAnimation *anim) {
	animation_ = anim;
	currentFrame_ = ((LightwaveAnimation *)animation_)->firstFrame_;
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
}

void AnimationWrapper::render() {
	if (!animation_) return;
	auto anim = (LightwaveAnimation *)animation_;
	for (auto &obj : anim->objects_) {
		if (obj.mesh) {
			auto &mvp = UniformBuffer::uniformBuffer<Matrices>("mvp");
			mvp.model = anim->calculateMatrix(obj, currentFrame_);
			UniformBuffer::updateUniformBuffer("mvp");
			obj.mesh->render();
		}
	}
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