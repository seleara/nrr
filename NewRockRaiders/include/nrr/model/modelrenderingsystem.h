#pragma once

#include <nrr/ecs/ecs.h>

#include <nrr/math/transform.h>
#include <nrr/math/time.h>
#include <nrr/math/uniformbuffer.h>
#include <nrr/model/model.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

class ModelComponent : public Component<ModelComponent>, public ModelWrapper {
public:
	ModelComponent() : Component<ModelComponent>(), ModelWrapper() {}
	ModelComponent(WadArchive &archive, const std::string &path, const std::string &anim) : Component<ModelComponent>(), ModelWrapper() {
		load(archive, path);
		play(anim);
	}
};

class ModelRenderingSystem : public System<ModelRenderingSystem> {
public:
	void fixedUpdate() override {
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

	void update() override {
		forEach<TransformComponent, ModelComponent>([&](TransformComponent &t, ModelComponent &m) {
			m.animation_.update();
		});
	}

	void render() override {
		static const std::string modelShaderName = "model";
		static const std::string whiteTextureName = "white";

		Shader shader;
		shader.loadCache(modelShaderName);
		shader.bind();

		Texture whiteTexture;
		whiteTexture.loadCache(whiteTextureName);

		forEach<TransformComponent, ModelComponent>([&](TransformComponent &t, ModelComponent &m) {
			if (!m.animation_.animation_) return;
			auto anim = (LightwaveAnimation *)m.animation_.animation_;
			/*for (auto &obj : anim->objects_) {
				if (obj.mesh) {
					auto &mvp = UniformBuffer::uniformBuffer<Matrices>("mvp");
					mvp.model = anim->calculateMatrix(obj, m.animation_.currentFrame_);
					auto trans = glm::translate(glm::mat4(), t.position);
					mvp.model = trans * mvp.model;
					// Rotation and scale later
					UniformBuffer::updateUniformBuffer("mvp");
					((LightwaveMesh *)obj.mesh)->render(m.animation_.currentFrame_);
				}
			}*/
			auto mvp = UniformBuffer::uniformBuffer<Matrices>("mvp");
			for (int i = 0; i < anim->objects_.size(); ++i) {
				auto &obj = anim->objects_[i];
				if (obj.mesh) {
					mvp->model = m.animation_.matrices_[i].matrix;
					auto scale = glm::scale(glm::mat4(), t.scale * m.transform().scale);
					auto trans = glm::translate(glm::mat4(), t.position);
					auto rotate = glm::mat4_cast(t.rotation);
					mvp->model = trans * rotate * scale * mvp->model;
					mvp.update();
					((LightwaveMesh *)obj.mesh)->render(m.animation_.currentFrame_, shader, whiteTexture);
				}
			}
		});
	}
private:
};