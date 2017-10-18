#pragma once

#include <memory>

#include <nrr/ecs/components/component.h>
#include <nrr/resource/texture/texture.h>

enum class Anchor {
	TopLeft,
	Top,
	TopRight,
	Left,
	Center,
	Right,
	BottomLeft,
	Bottom,
	BottomRight
};

enum class Pivot {
	TopLeft,
	Top,
	TopRight,
	Left,
	Center,
	Right,
	BottomLeft,
	Bottom,
	BottomRight
};

struct Sprite {
	Texture texture;
	Anchor anchor;
	Pivot pivot;
	glm::ivec2 pivotOffset;
	glm::vec4 color = glm::vec4(1, 1, 1, 1);

	const glm::ivec2 calcPivot() const {
		const auto tx = texture.size().x;
		const auto ty = texture.size().y;
		switch (pivot) {
		case Pivot::TopLeft:
			return pivotOffset;
		case Pivot::Top:
			return glm::ivec2(tx / 2 + pivotOffset.x, pivotOffset.y);
		case Pivot::TopRight:
			return glm::ivec2(tx - pivotOffset.x, pivotOffset.y);
		case Pivot::Left:
			return glm::ivec2(pivotOffset.x, ty / 2 + pivotOffset.y);
		case Pivot::Center:
			return glm::ivec2(tx / 2 + pivotOffset.x, ty / 2 + pivotOffset.y);
		case Pivot::Right:
			return glm::ivec2(tx - pivotOffset.x, ty / 2 + pivotOffset.y);
		case Pivot::BottomLeft:
			return glm::ivec2(pivotOffset.x, ty - pivotOffset.y);
		case Pivot::Bottom:
			return glm::ivec2(tx / 2 + pivotOffset.x, ty - pivotOffset.y);
		case Pivot::BottomRight:
			return glm::ivec2(tx - pivotOffset.x, ty - pivotOffset.y);
		}
	}

	const glm::vec4 rect() const {
		return glm::vec4(0, 0, texture.size().x, texture.size().y);
	}
};

struct SpriteComponent : public Component<Sprite>, public Sprite {

};