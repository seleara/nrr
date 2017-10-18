#include <nrr/model/model.h>

#include <nrr/model/formats/lightwave.h>
#include <nrr/resource/shader/shader.h>

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
	/*for (auto &kv : meshes_) {
		auto &m = *kv.second.get();
		m.render();
	}*/
	for (auto &kv : animations_) {
		auto &anim = *kv.second.get();
		anim.render();
	}
}

void ModelWrapper::load(WadArchive &archive, const std::string &path) {
	resource_ = LightwaveModelLoader::load(archive, path);
}

void ModelWrapper::fixedUpdate() {
	resource_->fixedUpdate();
}

void ModelWrapper::update() {
	resource_->update();
}

void ModelWrapper::render() {
	resource_->render();
}