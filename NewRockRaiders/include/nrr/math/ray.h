#pragma once

#include <glm/glm.hpp>

class Ray {
public:
	Ray() : near_(0, 0, 0), far_(0, 0, 1) {}
	Ray(glm::vec3 near, glm::vec3 far) : near_(near), far_(far) {}

	bool intersects(const glm::vec3(&triangle)[3], glm::vec3 &point) const;
private:
	glm::vec3 near_;
	glm::vec3 far_;
};