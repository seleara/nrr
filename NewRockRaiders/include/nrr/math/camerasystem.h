#pragma once

#include <glm/glm.hpp>

#include <nrr/ecs/ecs.h>

struct WindowEvent;

class CameraSystem : public System<CameraSystem> {
public:
	void update() override;
	void handleEvent(WindowEvent &event) override;
private:
	glm::vec3 cameraVelocity_;
	int cameraRotation_;
};