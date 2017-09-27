#include <nrr/ecs/entities/entity.h>

#include <utility>

Entity::Entity(EntityID id, EntityManager *manager) : id_(id), manager_(manager) {

}

Entity::Entity(const Entity &entity) : id_(entity.id_), manager_(entity.manager_) {

}

Entity::Entity(Entity &&entity) : id_(std::move(entity.id_)), manager_(std::move(entity.manager_)) {

}

Entity &Entity::operator=(const Entity &other) {
	id_ = other.id_;
	manager_ = other.manager_;
	return *this;
}
