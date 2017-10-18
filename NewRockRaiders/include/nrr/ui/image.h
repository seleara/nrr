#pragma once

#include <nrr/ecs/ecs.h>
#include <nrr/resource/texture/texture.h>
#include <nrr/ui/widget.h>

struct UiImage : public Component<UiImage>, public Widget {
	Texture texture;

};