#pragma once

#include <nrr/ecs/systems/system.h>
#include <nrr/sprite/spritebatch.h>

class SpriteRenderingSystem : public System<SpriteRenderingSystem> {
public:
	void update();
	void render();
private:
	SpriteBatch batch_;
};