#pragma once

#include <nrr/ecs/ecs.h>
#include <nrr/level/level.h>

class LevelSystem : public System<LevelSystem> {
public:
	void update() override;
	void fixedUpdate() override;
	void render() override;
private:
};