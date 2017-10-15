#pragma once

#include <set>
#include <map>
#include <memory>
#include <vector>

#include <nrr/ecs/common/defines.h>
#include <nrr/ecs/common/pool.h>
#include <nrr/ecs/entities/entity.h>
#include <nrr/ecs/components/component.h>
#include <nrr/ecs/systems/basesystem.h>

class EntityManager {
public:
	~EntityManager() {
		for (auto *s : systems_) {
			delete s;
		}
		for (auto &kv : componentPools_) {
			delete kv.second;
		}
	}

	Entity wrap(EntityID id) {
		Entity e(id, this);
		return e;
	}

	Entity create() {
		Entity e(entityCounter_++, this);
		return e;
	}

	Entity destroy(EntityID id) {
		for (auto cid : entityComponents_[id]) {
			componentPools_[cid]->destroy(id);
			componentsToEntities_[cid].erase(id);
		}
		entityComponents_.erase(id);
	}

	template <typename S>
	void registerSystem() {
		auto *system = new S();
		system->manager_ = this;
		systems_.push_back(system);
	}

	void update() {
		for (auto *s : systems_) {
			s->update();
		}
	}

	void fixedUpdate() {
		for (auto *s : systems_) {
			s->fixedUpdate();
		}
	}

	void render() {
		for (auto *s : systems_) {
			s->render();
		}
	}

	template <typename C, typename ... Args>
	ComponentWrapper<C> add(EntityID id, Args && ... args) {
		componentsToEntities_[C::id()].insert(id);
		Pool<C> *pool;
		auto iter = componentPools_.find(C::id());
		if (iter == componentPools_.end()) {
			pool = new Pool<C>(8192);
			pool->allocate(4);
			componentPools_[C::id()] = pool;
		} else {
			pool = (Pool<C> *)iter->second;
		}
		entityComponents_[id].push_back(C::id());
		::new(pool->get(id)) C(std::forward<Args>(args)...);
		ComponentWrapper<C> wrapper(this, id);
		return wrapper;
	}

	template <typename C>
	ComponentWrapper<C> get(EntityID id) {
		ComponentWrapper<C> wrapper(this, id);
		return wrapper;
	}

	template <typename C>
	C *getPointer(EntityID id) {
		C *ptr = (C *)((Pool<C> *)componentPools_[C::id()])->get(id);
		return ptr;
	}

	template <typename C>
	const C *getPointer(EntityID id) const {
		auto iter = componentPools_.find(C::id());
		return iter->second->get(id);
	}

	template <typename C>
	std::set<EntityID> allWith() {
		return componentsToEntities_[C::id()];
	}
private:
	friend class Entity;

	size_t entityCounter_ = 0;

	std::vector<BaseSystem *> systems_;
	std::map<ComponentID, BasePool *> componentPools_;
	std::map<ComponentID, std::set<EntityID>> componentsToEntities_;
	std::map<EntityID, std::vector<ComponentID>> entityComponents_;
};