#pragma once

#include <memory>

class WindowEvent;

class EntityManager;

class BaseSystem {
public:
	virtual ~BaseSystem() {}
	virtual void update() {}
	virtual void fixedUpdate() {}
	virtual void render() {}
	virtual void render2d() {}
	virtual void handleEvent(WindowEvent &event) {}
protected:
	friend class EntityManager;
	EntityManager *manager_;
};