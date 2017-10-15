#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <nrr/ecs/components/component.h>

struct Transform : public Component<Transform> {
	glm::vec3 position;
	glm::vec3 scale = glm::vec3(1, 1, 1);
	glm::quat rotation;
};