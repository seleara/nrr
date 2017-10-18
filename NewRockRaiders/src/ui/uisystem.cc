#include <nrr/ui/uisystem.h>

#include <nrr/sprite/sprite.h>
#include <nrr/sprite/spritebatch.h>
#include <nrr/math/transform.h>
#include <nrr/ui/image.h>

void UiSystem::render() {
	SpriteBatch batch;
	forEach<TransformComponent, UiImage>([&](TransformComponent &t, UiImage &i) {
		Sprite sprite;
		sprite.texture = i.texture;
		//batch.add(sprite, t);
	});
	batch.render();
}