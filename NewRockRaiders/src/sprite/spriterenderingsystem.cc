#include <nrr/sprite/spriterenderingsystem.h>

#include <iostream>

#include <nrr/math/transform.h>
#include <nrr/sprite/sprite.h>

void SpriteRenderingSystem::update() {
	batch_.clear();
	//std::cout << "Start update.\n";
	forEach<Transform, Sprite>([&](Transform &t, Sprite &s) {
		batch_.add(s, t);
		//std::cout << "Adding sprite.\n";
	});
	//std::cout << "Done with update." << std::endl;
}

void SpriteRenderingSystem::render() {
	std::cout << "Render: " << batch_.size() << "\n";
	batch_.render();
}