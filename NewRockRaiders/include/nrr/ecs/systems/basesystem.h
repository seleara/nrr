#pragma once

#include <memory>

class EntityManager;

class BaseSystem {
public:
	virtual ~BaseSystem() {}
	virtual void update() {}
	virtual void fixedUpdate() {}
	virtual void render() {}
protected:
	friend class EntityManager;
	EntityManager *manager_;
};