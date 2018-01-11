#include <nrr/model/modelrenderingsystem.h>

#include <nrr/math/transform.h>
#include <nrr/math/time.h>
#include <nrr/math/uniformbuffer.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

void ModelRenderingSystem::fixedUpdate() {
	forEach<ModelComponent>([&](ModelComponent &m) {
		/*if (!m.animation_.animation_) return;
		auto anim = (LightwaveAnimation *)m.animation_.animation_;
		m.animation_.time_ += Time::fixedDeltaTime();
		while (m.animation_.time_ >= (1.0 / anim->fps_)) {
		m.animation_.time_ -= (1.0 / anim->fps_);
		++m.animation_.currentFrame_;
		if (m.animation_.currentFrame_ > anim->lastFrame_) {
		m.animation_.currentFrame_ = anim->firstFrame_;
		}
		++m.animation_.currentSequenceFrame_;
		//if (surf.ctex.sequenceFrame >= surf.ctex.textures.size()) {
		//	surf.ctex.sequenceFrame = 0;
		//}
		}*/
		m.animation_.fixedUpdate();
	});
}

void ModelRenderingSystem::update() {
	forEach<TransformComponent, ModelComponent>([&](TransformComponent &t, ModelComponent &m) {
		m.animation_.update();
	});
}

void ModelRenderingSystem::render() {
	static const std::string modelShaderName = "model";
	static const std::string whiteTextureName = "white";

	Shader shader;
	shader.loadCache(modelShaderName);
	shader.bind();

	Texture whiteTexture;
	whiteTexture.loadCache(whiteTextureName);

	std::map<ModelComponent *, std::vector<glm::mat4>> matCache;

	// Render opaque surfaces
	forEach<TransformComponent, ModelComponent>([&](TransformComponent &t, ModelComponent &m) {
		if (!m.animation_.animation_) return;
		auto anim = (LightwaveAnimation *)m.animation_.animation_;
		auto mvp = UniformBuffer::uniformBuffer<Matrices>("mvp");
		for (int i = 0; i < anim->objects_.size(); ++i) {
			auto &obj = anim->objects_[i];
			if (obj.mesh) {
				mvp->model = m.animation_.matrices_[i].matrix;
				auto scale = glm::scale(glm::mat4(), t.scale * m.transform().scale * glm::vec3(-1, 1, 1));
				auto trans = glm::translate(glm::mat4(), t.position);
				auto rotate = glm::mat4_cast(t.rotation);
				mvp->model = trans * rotate * scale * mvp->model;
				matCache[&m].push_back(mvp->model);
				mvp.update();
				((LightwaveMesh *)obj.mesh)->renderOpaque(m.animation_.currentFrame_, shader, whiteTexture);
			} else {
				matCache[&m].push_back(glm::mat4());
			}
		}
	});

	// Render additive surfaces
	// Since depth writing must be turned off for additive surfaces to avoid overwriting color in the backbuffer
	// with a transparent part of the surface, they must be rendered after all other opaque objects have been rendered
	// TODO: If there is a need for some other kind of rendering apart from models, this might have to be split into a
	// new method. Working for now though
	forEach<TransformComponent, ModelComponent>([&](TransformComponent &t, ModelComponent &m) {
		if (!m.animation_.animation_) return;
		auto anim = (LightwaveAnimation *)m.animation_.animation_;
		auto mvp = UniformBuffer::uniformBuffer<Matrices>("mvp");
		for (int i = 0; i < anim->objects_.size(); ++i) {
			auto &obj = anim->objects_[i];
			if (obj.mesh) {
				const auto &cache = matCache[&m];
				mvp->model = cache[i];
				mvp.update();
				((LightwaveMesh *)obj.mesh)->renderAdditive(m.animation_.currentFrame_, shader, whiteTexture);
			}
		}
	});
}