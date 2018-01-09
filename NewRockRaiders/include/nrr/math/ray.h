#pragma once

#include <glm/glm.hpp>

class Ray {
public:
	Ray() : near_(0, 0, 0), far_(0, 0, 1) {}
	Ray(glm::vec3 near, glm::vec3 far) : near_(near), far_(far) {}

	bool intersects(const glm::vec3 (&triangle)[3], glm::vec3 &point) {
		glm::vec3 normal, intersectPosition;

		normal = glm::cross(triangle[1] - triangle[0], triangle[2] - triangle[0]);
		normal = glm::normalize(normal);

		float dist1 = glm::dot(near_ - triangle[0], normal);
		float dist2 = glm::dot(far_ - triangle[0], normal);

		if (dist1 * dist2 >= 0.0f) {
			return false;
		}

		if (dist1 == dist2) {
			return false;
		}

		intersectPosition = near_ + (far_ - near_) * (-dist1 / (dist2 - dist1));

		glm::vec3 vTest;

		vTest = glm::cross(normal, triangle[1] - triangle[0]);
		if (glm::dot(vTest, intersectPosition - triangle[0]) < 0.0f) {
			return false;
		}

		vTest = glm::cross(normal, triangle[2] - triangle[1]);
		if (glm::dot(vTest, intersectPosition - triangle[1]) < 0.0f) {
			return false;
		}

		vTest = glm::cross(normal, triangle[0] - triangle[2]);
		if (glm::dot(vTest, intersectPosition - triangle[0]) < 0.0f) {
			return false;
		}

		point = intersectPosition;

		return true;
	}
private:
	glm::vec3 near_;
	glm::vec3 far_;
};