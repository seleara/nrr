#pragma once

#include <bitset>
#include <vector>

#include <nrr/ecs/ecs.h>
#include <nrr/math/vertexbuffer.h>
#include <nrr/resource/texture/texture.h>

enum class WallType {
	Dirt = 0,
	LooseRock,
	HardRock,
	SolidRock,
	EnergyCrystalSeam,
	OreSeam,
	RechargeSeam
};

enum class WallOrientation {
	Invalid = 0,
	Roof,
	StraightN,
	StraightE,
	StraightS,
	StraightW,
	EdgeNW,
	EdgeNE,
	EdgeSE,
	EdgeSW,
	CornerNW,
	CornerNE,
	CornerSE,
	CornerSW,
	Inbetween0,
	Inbetween1
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
	PowerPath,
	BuildingPowerPath
};

struct Tile {
	bool initialized = false;
	bool visible = true;
	bool wall;
	WallType wallType;
	WallOrientation wallOrientation;
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
	bool powered = false;

	glm::vec4 highlightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	bool dirty = false;

	bool operator==(const Tile &other) const {
		bool ret = initialized == other.initialized;
		ret &= visible == other.visible;
		ret &= wall == other.wall;
		ret &= wallType == other.wallType;
		ret &= wallOrientation == other.wallOrientation;
		ret &= groundType == other.groundType;
		ret &= drillTime == other.drillTime;
		ret &= oreCount == other.oreCount;
		ret &= energyCrystalCount == other.energyCrystalCount;
		ret &= alwaysErode == other.alwaysErode;
		ret &= erosionSpeed == other.erosionSpeed;
		ret &= landslide == other.landslide;
		ret &= landslideStrength == other.landslideStrength;
		ret &= pathType == other.pathType;
		ret &= rubble == other.rubble;
		ret &= height == other.height;
		ret &= highlightColor == other.highlightColor;
		ret &= powered == other.powered;
		return ret;
	}

	bool operator!=(const Tile &other) const {
		return !(*this == other);
	}
};

struct TileHeights {
	float topLeft, topRight, bottomRight, bottomLeft;
};

class ConfigParser;
class WadArchive;

struct LevelVertex {
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
};

struct Selection {
	enum class Type {
		Tile,
		Raider,
		Vehicle,
		Building,
	};

	Tile *selectedTile;
};

struct Level : public Component<Level> {
	void load(EntityManager &em, ConfigParser &config, WadArchive &archive, int level);
	inline int tileIndex(int x, int y) const {
		return (y * size.x + x);
	}
	inline int tileVertexIndex(int x, int y) const {
		return tileIndex(x, y) * 6;
	}
	Tile *tile(int x, int y) {
		if (x >= 0 && x < size.x && y >= 0 && y < size.y) return &tiles[tileIndex(x, y)];
		return nullptr;
	}
	glm::ivec2 size;
	float blockSize;
	std::vector<Tile> tiles;
	VertexBuffer<float> buffer;
	GLuint vao;
	std::vector<LevelVertex> vertices;
	Texture atlas;

	Selection selection;
	
	// Texture Definitions
	float roofUnit;
	std::string roofTexture, meshBaseName, textureBaseName;
private:
	friend class LevelSystem;
	bool updateTile(int x, int y);
	void updateTileVertices(int x, int y);
	void uploadTiles();
	void uploadTile(int x, int y);

	int bitCount(int n) const {
		int count = 0;
		for (int i = 0; i < 32; ++i) {
			count += (n & (1 << i)) != 0;
		}
		return count;
	}

	bool wallMask(int walls, const char *mask) {
		for (int i = 0; i < 8; ++i) {
			switch (mask[i]) {
			case ' ': // Anything
				continue;
			case '.': // Ground
				if (((walls >> i) & 1) != 0) return false;
				break;
			case '#': // Wall
				if (((walls >> i) & 1) != 1) return false;
				break;
			default:
				throw std::runtime_error("Invalid wall mask string.");
			}
		}
		//std::cout << std::bitset<8>(walls) << " matched " << mask << ".\n";
		return true;
	}

	template <int TriangleDirection, std::size_t N>
	void setPoints(LevelVertex *lv, int x, int y, const int (&heights)[N], const TileHeights &th) {
		static_assert(N == 4);
		static_assert(TriangleDirection == 0 || TriangleDirection == 1);

		float left = x;
		float right = left + 1;
		float top = size.y - y;
		float bottom = top + 1;

		switch (TriangleDirection) {
		case 0:
			(lv++)->position = glm::vec3(left, heights[0], top) * blockSize + glm::vec3(0, th.topLeft, 0);
			(lv++)->position = glm::vec3(left, heights[3], bottom) * blockSize + glm::vec3(0, th.bottomLeft, 0);
			(lv++)->position = glm::vec3(right, heights[2], bottom) * blockSize + glm::vec3(0, th.bottomRight, 0);
			(lv++)->position = glm::vec3(right, heights[2], bottom) * blockSize + glm::vec3(0, th.bottomRight, 0);
			(lv++)->position = glm::vec3(right, heights[1], top) * blockSize + glm::vec3(0, th.topRight, 0);
			(lv++)->position = glm::vec3(left, heights[0], top) * blockSize + glm::vec3(0, th.topLeft, 0);
			break;
		case 1:
			(lv++)->position = glm::vec3(left, heights[3], bottom) * blockSize + glm::vec3(0, th.bottomLeft, 0);
			(lv++)->position = glm::vec3(right, heights[2], bottom) * blockSize + glm::vec3(0, th.bottomRight, 0);
			(lv++)->position = glm::vec3(right, heights[1], top) * blockSize + glm::vec3(0, th.topRight, 0);
			(lv++)->position = glm::vec3(right, heights[1], top) * blockSize + glm::vec3(0, th.topRight, 0);
			(lv++)->position = glm::vec3(left, heights[0], top) * blockSize + glm::vec3(0, th.topLeft, 0);
			(lv++)->position = glm::vec3(left, heights[3], bottom) * blockSize + glm::vec3(0, th.bottomLeft, 0);
			break;
		}
	}

	void setColor(LevelVertex *lv, const glm::vec4 &color) {
		(lv++)->color = color;
		(lv++)->color = color;
		(lv++)->color = color;
		(lv++)->color = color;
		(lv++)->color = color;
		(lv++)->color = color;
	}
	
	template <int TriangleDirection, std::size_t N>
		void setColors(LevelVertex *lv, const glm::vec4(&colors)[N]) {
		static_assert(N == 4);

		switch (TriangleDirection) {
		case 0:
			(lv++)->color = colors[0];
			(lv++)->color = colors[3];
			(lv++)->color = colors[2];
			(lv++)->color = colors[2];
			(lv++)->color = colors[1];
			(lv++)->color = colors[0];
		break;
		case 1:
			(lv++)->color = colors[3];
			(lv++)->color = colors[2];
			(lv++)->color = colors[1];
			(lv++)->color = colors[1];
			(lv++)->color = colors[0];
			(lv++)->color = colors[3];
			break;
		}
	}

	template <int TextureRotation>
	void setUVs(LevelVertex *lv, int textureIndex) {
		static_assert(TextureRotation >= 0 && TextureRotation < 4);
		float unit = textureIndex == 0 ? roofUnit : 0.0625f;
		int x = textureIndex % 16;
		int y = textureIndex / 16;
		float left = unit * x;
		float right = left + unit;
		float top = unit * y;
		float bottom = top + unit;

		switch (TextureRotation) {
		case 0:
			(lv++)->uv = glm::vec2(left, top);
			(lv++)->uv = glm::vec2(left, bottom);
			(lv++)->uv = glm::vec2(right, bottom);
			(lv++)->uv = glm::vec2(right, bottom);
			(lv++)->uv = glm::vec2(right, top);
			(lv++)->uv = glm::vec2(left, top);
			break;
		case 1:
			(lv++)->uv = glm::vec2(left, bottom);
			(lv++)->uv = glm::vec2(right, bottom);
			(lv++)->uv = glm::vec2(right, top);
			(lv++)->uv = glm::vec2(right, top);
			(lv++)->uv = glm::vec2(left, top);
			(lv++)->uv = glm::vec2(left, bottom);
			break;
		case 2:
			(lv++)->uv = glm::vec2(right, bottom);
			(lv++)->uv = glm::vec2(right, top);
			(lv++)->uv = glm::vec2(left, top);
			(lv++)->uv = glm::vec2(left, top);
			(lv++)->uv = glm::vec2(left, bottom);
			(lv++)->uv = glm::vec2(right, bottom);
			break;
		case 3:
			(lv++)->uv = glm::vec2(right, top);
			(lv++)->uv = glm::vec2(left, top);
			(lv++)->uv = glm::vec2(left, bottom);
			(lv++)->uv = glm::vec2(left, bottom);
			(lv++)->uv = glm::vec2(right, bottom);
			(lv++)->uv = glm::vec2(right, top);
			break;
		}
	}
	void setup(WadArchive &archive);
};