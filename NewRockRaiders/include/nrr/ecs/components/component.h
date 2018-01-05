#pragma once

#include <nrr/ecs/components/basecomponent.h>
#include <nrr/ecs/entities/entity.h>

template <typename C>
struct Component : public BaseComponent {
	static ComponentID id() {
		static ComponentID classId = BaseComponent::id();
		return classId;
	}
};

class EntityManager;

template <typename C>
class ComponentWrapper {
public:
	ComponentWrapper() : manager_(nullptr), id_(0) {}
	ComponentWrapper(EntityManager *manager, EntityID id) : manager_(manager), id_(id) {}
	ComponentWrapper(const ComponentWrapper &other) : manager_(other.manager_), id_(other.id_) {}
	ComponentWrapper(ComponentWrapper &&other) : manager_(std::move(other.manager_)), id_(std::move(other.id_)) {}
	ComponentWrapper &operator=(const ComponentWrapper &other) {
		manager_ = other.manager_;
		id_ = other.id_;
		return *this;
	}

	Entity entity() const {
		return manager_->wrap(id_);
	}

	C *ptr();
	const C *ptr() const;

	C &get();
	const C &get() const;

	C *operator->();
private:
	EntityManager *manager_;
	EntityID id_;
};

#include <nrr/ecs/entities/entitymanager.h>

template <typename C>
C *ComponentWrapper<C>::ptr() {
	return manager_->getPointer<C>(id_);
}

template <typename C>
const C *ComponentWrapper<C>::ptr() const {
	return manager_->getPointer<C>(id_);
}

template <typename C>
C &ComponentWrapper<C>::get() {
	return *ptr();
}

template <typename C>
const C &ComponentWrapper<C>::get() const {
	return *ptr();
}

template <typename C>
C *ComponentWrapper<C>::operator->() {
	return manager_->getPointer<C>(id_);
}