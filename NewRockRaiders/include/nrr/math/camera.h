#pragma once

#include <glm/glm.hpp>

#include <nrr/ecs/ecs.h>

enum class CameraMode {
	Free,
	Target
};

struct CameraComponent : public Component<CameraComponent> {
	CameraMode mode = CameraMode::Free;
	glm::vec3 target;

	static CameraComponent *main;
};