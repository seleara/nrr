#pragma once

#include <nrr/ecs/common/defines.h>
#include <nrr/ecs/common/pool.h>
#include <nrr/ecs/entities/entity.h>
#include <nrr/ecs/components/component.h>

class EntityManager {
public:
	Entity wrap(EntityID id);

	Entity create();


private:
	friend class Entity;
};