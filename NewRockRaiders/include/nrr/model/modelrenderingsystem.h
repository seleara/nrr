#pragma once

#include <nrr/ecs/ecs.h>

#include <nrr/math/transform.h>
#include <nrr/math/time.h>
#include <nrr/math/uniformbuffer.h>
#include <nrr/model/model.h>

#include <glm/gtc/matrix_transform.hpp>>

class ModelComponent : public Component<ModelComponent>, public ModelWrapper {
};

class ModelRenderingSystem : public System<ModelRenderingSystem> {
public:
	void fixedUpdate() override {
		forEach<ModelComponent>([&](ModelComponent &m) {
			if (!m.animation_.animation_) return;
			auto anim = (LightwaveAnimation *)m.animation_.animation_;
			m.animation_.time_ += Time::fixedDeltaTime();
			while (m.animation_.time_ >= (1.0 / anim->fps_)) {
				m.animation_.time_ -= (1.0 / anim->fps_);
				++m.animation_.currentFrame_;
				if (m.animation_.currentFrame_ > anim->lastFrame_) {
					m.animation_.currentFrame_ = anim->firstFrame_;
				}
				/*++m.animation_.currentSequenceFrame_;
				if (surf.ctex.sequenceFrame >= surf.ctex.textures.size()) {
					surf.ctex.sequenceFrame = 0;
				}*/
			}
		});
	}

	void render() override {
		forEach<TransformComponent, ModelComponent>([&](TransformComponent &t, ModelComponent &m) {
			if (!m.animation_.animation_) return;
			auto anim = (LightwaveAnimation *)m.animation_.animation_;
			for (auto &obj : anim->objects_) {
				if (obj.mesh) {
					auto &mvp = UniformBuffer::uniformBuffer<Matrices>("mvp");
					mvp.model = anim->calculateMatrix(obj, m.animation_.currentFrame_);
					auto trans = glm::translate(glm::mat4(), t.position);
					mvp.model = trans * mvp.model;
					// Rotation and scale later
					UniformBuffer::updateUniformBuffer("mvp");
					((LightwaveMesh *)obj.mesh)->render(m.animation_.currentFrame_);
				}
			}
		});
	}
private:
};