#pragma once

#include <algorithm>
#include <functional>
#include <iterator>
#include <set>

#include <nrr/ecs/common/defines.h>
#include <nrr/ecs/entities/entitymanager.h>
#include <nrr/ecs/systems/basesystem.h>

template <typename S>
class System : public BaseSystem {
public:
	virtual ~System() {}
protected:
	template <typename ... Args>
	void forEach(std::function<void(Args & ...)> func) {
		std::set<EntityID> finalSet = allWith<Args...>();
		for (auto id : finalSet) {
			//buildFunction<Args...>(func, id);
			func(*(manager_->getPointer<Args>(id))...);
		}
	}

	template <typename C>
	void buildFunction(std::function<void(C &)> func, EntityID id) {
		func(manager_->get<C>(id).get());
	}

	template <typename C, typename C2, typename ... Args>
	void buildFunction(std::function<void(C &, C2 &, Args & ...)> func, EntityID id) {
		std::function<void(C2 &, Args & ...)> g = [&](C2 &c2, Args & ... args) { return std::move(func)(std::move(manager_->get<C>(id).get()), std::forward<C2>(c2), std::forward<Args>(args)...); };
		buildFunction<Args...>(g, id);
	}

	std::set<EntityID> allWith() {
		std::set<EntityID> ids;
		return ids;
	}

	template <typename C>
	std::set<EntityID> allWith() {
		std::set<EntityID> ids = manager_->allWith<C>();
		return ids;
	}

	template <typename C, typename C2, typename ... Args>
	std::set<EntityID> allWith() {
		std::set<EntityID> ids = manager_->allWith<C>();
		//std::cout << "Start = " << ids.size() << "\n";
		allWith_<C2, Args...>(ids);
		return ids;
	}

	template <typename C>
	void allWith_(std::set<EntityID> &ids) {
		std::set<EntityID> intersect;
		auto s1 = manager_->allWith<C>();
		std::set_intersection(s1.begin(), s1.end(), ids.begin(), ids.end(), std::inserter(intersect, intersect.end()));
		ids = intersect;
		//std::cout << "    " << ids.size() << "\n";
	}

	template <typename C, typename C2, typename ... Args>
	void allWith_(std::set<EntityID> &ids) {
		std::set<EntityID> intersect;
		auto s1 = manager_->allWith<C>();
		std::set_intersection(s1.begin(), s1.end(), ids.begin(), ids.end(), std::inserter(intersect, intersect.end()));
		ids = intersect;
		//std::cout << "    " << ids.size() << "\n";
		allWith_<C2, Args...>(ids);
	}
	
private:
};