#pragma once

#include <vector>

#include <nrr/ecs/ecs.h>
#include <nrr/math/vertexbuffer.h>
#include <nrr/resource/texture/texture.h>

enum class WallType {
	Dirt,
	LooseRock,
	HardRock,
	SolidRock,
	EnergyCrystalSeam,
	OreSeam,
	RechargeSeam
};

enum class GroundType {
	Ground,
	Lava,
	Water,
	SlugHole
};

enum class PathType {
	None,
	Rubble,
	PowerPath
};

struct Tile {
	bool visible;
	bool wall;
	WallType wallType;
	GroundType groundType;
	float drillTime;
	int oreCount;
	int energyCrystalCount;
	bool alwaysErode;
	float erosionSpeed;
	bool landslide;
	float landslideStrength;
	PathType pathType;
	float rubble;
	float height;
};

class ConfigParser;
class WadArchive;

struct LevelVertex {
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
};

struct Level : public Component<Level> {
	void load(ConfigParser &config, WadArchive &archive, int level);
	Tile *tile(int x, int y) {
		if (x >= 0 && x < size.x && y >= 0 && y < size.y) return &tiles[y * size.x + x];
		return nullptr;
	}
	glm::ivec2 size;
	float blockSize;
	std::vector<Tile> tiles;
	VertexBuffer<float> buffer;
	GLuint vao;
	std::vector<LevelVertex> vertices;
	Texture atlas;
	
	// Texture Definitions
	float roofUnit;
	std::string roofTexture, meshBaseName, textureBaseName;
private:
	int bitCount(int n) const {
		int count = 0;
		for (int i = 0; i < 32; ++i) {
			count += (n & (1 << i)) != 0;
		}
		return count;
	}
	void setUVs(LevelVertex *lv, int textureIndex) {
		float unit = textureIndex == 0 ? roofUnit : 0.0625f;
		int x = textureIndex % 16;
		int y = textureIndex / 16;
		float left = unit * x;
		float right = left + unit;
		float top = unit * y;
		float bottom = top + unit;

		(lv++)->uv = glm::vec2(left, top);
		(lv++)->uv = glm::vec2(right, top);
		(lv++)->uv = glm::vec2(right, bottom);
		(lv++)->uv = glm::vec2(right, bottom);
		(lv++)->uv = glm::vec2(left, bottom);
		(lv++)->uv = glm::vec2(left, top);
	}
	void setup(WadArchive &archive);
};