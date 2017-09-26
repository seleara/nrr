#pragma once

#include <nrr/ecs/common/defines.h>

template <typename C>
class ComponentWrapper;

/**
 * Entity is a lightweight wrapper for a unique entity ID.
 */
class Entity {
public:
	Entity(const Entity &entity);

	Entity(Entity &&entity);

	Entity &operator=(const Entity &other);

	template <typename C, typename ...Args>
	ComponentWrapper<C> add(Args && ...args) {
		return manager_.add<C>(id_, ...args);
	}
private:
	friend class EntityManager;

	explicit Entity::Entity(EntityID id, EntityManager *manager);

	EntityID id_;
	EntityManager *manager_;
};