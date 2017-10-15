#pragma once

#include <nrr/ecs/components/basecomponent.h>

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
	ComponentWrapper(EntityManager *manager, EntityID id) : manager_(manager), id_(id) {}

	C &get();
	const C &get() const;

	C *operator->();
private:
	EntityManager *manager_;
	EntityID id_;
};

#include <nrr/ecs/entities/entitymanager.h>

template <typename C>
C &ComponentWrapper<C>::get() {
	return *manager_->getPointer<C>(id_);
}

template <typename C>
const C &ComponentWrapper<C>::get() const {
	return *manager_->getPointer<C>(id_);
}

template <typename C>
C *ComponentWrapper<C>::operator->() {
	return manager_->getPointer<C>(id_);
}