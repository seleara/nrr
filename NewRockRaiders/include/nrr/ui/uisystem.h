#pragma once

#include <nrr/ecs/systems/system.h>

class UiSystem : public System<UiSystem> {
public:
	void update() override;
	void render() override;
private:
};