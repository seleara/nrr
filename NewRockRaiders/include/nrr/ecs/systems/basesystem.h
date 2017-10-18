#pragma once

#include <memory>

class EntityManager;

class BaseSystem {
public:
	virtual ~BaseSystem() {}
	virtual void update() {}
	virtual void fixedUpdate() {}
	virtual void render() {}
	virtual void render2d() {}
protected:
	friend class EntityManager;
	EntityManager *manager_;
};