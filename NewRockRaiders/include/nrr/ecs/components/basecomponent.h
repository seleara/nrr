#pragma once

#include <nrr/ecs/common/defines.h>

struct BaseComponent {
	static ComponentID id() {
		static ComponentID classIdCounter = 0;
		return classIdCounter++;
	}
};