#pragma once

#include <nrr/ecs/ecs.h>
#include <nrr/level/level.h>

class LevelSystem : public System<LevelSystem> {
public:
	void init() override;
	void update() override;
	void fixedUpdate() override;
	void render() override;
private:
	Entity pick_;
};