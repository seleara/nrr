#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <nrr/ecs/ecs.h>

#include <nrr/math/transform.h>
#include <nrr/math/camera.h>
#include <nrr/math/uniformbuffer.h>

class CameraSystem : public System<CameraSystem> {
public:
	void update() override {
		auto mvp = UniformBuffer::uniformBuffer<Matrices>("mvp");
		forEach<TransformComponent, CameraComponent>([&](TransformComponent &t, CameraComponent &c) {
			//std::cout << "Camera test." << std::endl;
			if (CameraComponent::main == &c) {
				mvp->projection = glm::perspectiveFov(90.0f, 1280.0f, 720.0f, 0.1f, 1000.0f);
				if (c.mode == CameraMode::Target) {
					mvp->view = glm::lookAt(t.position, c.target, glm::vec3(0, 1, 0));
				} else {
					mvp->view = glm::toMat4(t.rotation);
					mvp->view = glm::translate(mvp->view, -t.position);
				}
				mvp.update();
				//UniformBuffer::updateUniformBuffer("mvp");
			}
		});
	}
};