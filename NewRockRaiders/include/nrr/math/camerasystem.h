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
		if (!Window::main().isFocused())
			return;
		auto mvp = UniformBuffer::uniformBuffer<Matrices>("mvp");
		/*forEach<TransformComponent, CameraComponent>([&](TransformComponent &t, CameraComponent &c) {
			//std::cout << "Camera test." << std::endl;
			if (CameraComponent::main == c) {
				auto w = Window::main().size().x;
				auto h = Window::main().size().y;
				mvp->projection = glm::perspectiveFov(90.0f, w, h, 0.1f, 1000.0f);
				if (c.mode == CameraMode::Target) {
					mvp->view = glm::lookAt(t.position, c.target, glm::vec3(0, 1, 0));
				} else {
					mvp->view = glm::toMat4(t.rotation);
					mvp->view = glm::translate(mvp->view, -t.position);
				}
				mvp.update();
			}
		});*/

		// Input
		auto mpos = Window::main().mousePosition();
		if (mpos.x <= 0) {
			cameraVelocity_.x = -1;
		} else if (mpos.x >= Window::main().size().x - 1) {
			cameraVelocity_.x = 1;
		} else {
			cameraVelocity_.x = 0;
		}
		if (mpos.y <= 0) {
			cameraVelocity_.z = -1;
		} else if (mpos.y >= Window::main().size().y - 1) {
			cameraVelocity_.z = 1;
		} else {
			cameraVelocity_.z = 0;
		}

		// Update
		auto &c = CameraComponent::main;
		auto &t = c.entity().get<TransformComponent>();
		t->rotation = glm::rotate(t->rotation, cameraRotation_ * (float)Time::deltaTime(), glm::vec3(0, 1, 0));
		t->position += glm::inverse(t->rotation) * cameraVelocity_ * 160.0f * (float)Time::deltaTime();
		auto w = Window::main().size().x;
		auto h = Window::main().size().y;
		mvp->projection = glm::perspectiveFov(45.0f, w, h, 0.1f, 1000.0f);
		if (c->mode == CameraMode::Target) {
			mvp->view = glm::lookAt(t->position, c->target, glm::vec3(0, 1, 0));
		} else {
			mvp->view = glm::toMat4(glm::rotate(glm::quat(), 45.0f, glm::vec3(1, 0, 0)) * t->rotation);
			mvp->view = glm::translate(mvp->view, -t->position);
		}
		mvp.update();
	}

	void handleEvent(WindowEvent &event) override {
		/*auto &c = CameraComponent::main;
		auto &t = c.entity().get<TransformComponent>();
		if (event.type == WindowEvent::Type::MouseMoved) {
			if (event.position.x <= 0) {
				cameraVelocity_.x = -1;
			} else if (event.position.x >= Window::main().size().x) {
				cameraVelocity_.x = 1;
			} else {
				cameraVelocity_.x = 0;
			}
			if (event.position.y <= 0) {
				cameraVelocity_.z = -1;
			} else if (event.position.y >= Window::main().size().y) {
				cameraVelocity_.z = 1;
			} else {
				cameraVelocity_.z = 0;
			}
		}*/
		if (event.type == WindowEvent::Type::KeyPressed) {
			if (event.key == KeyCode::Q) {
				cameraRotation_ -= 1;
			} else if (event.key == KeyCode::E) {
				cameraRotation_ += 1;
			}
		} else if (event.type == WindowEvent::Type::KeyReleased) {
			if (event.key == KeyCode::Q) {
				cameraRotation_ += 1;
			} else if (event.key == KeyCode::E) {
				cameraRotation_ -= 1;
			}
		}
	}
private:
	glm::vec3 cameraVelocity_;
	int cameraRotation_;
};