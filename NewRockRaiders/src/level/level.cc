#include <nrr/level/level.h>

#include <fstream>
#include <sstream>
#include <iomanip>

#include <nrr/level/map.h>
#include <nrr/util/binaryreader.h>
#include <nrr/util/configparser.h>

void Level::load(ConfigParser &config, WadArchive &archive, int level) {
	std::stringstream ss;
	ss << "Lego*/Levels/Level" << std::setfill('0') << std::setw(2) << level << "/";
	auto levelBlock = ss.str();

	blockSize = config.get<float>(levelBlock + "BlockSize");

	auto textureSet = config.get(levelBlock + "TextureSet");
	auto textureBlock = "Lego*/" + textureSet + "/";
	roofTexture = config.get(textureBlock + "rooftexture");
	meshBaseName = config.get(textureBlock + "meshbasename");
	textureBaseName = config.get(textureBlock + "texturebasename");

	auto terrainPath = config.get(levelBlock + "TerrainMap");
	Map terrain;
	terrain.load(archive, terrainPath);
	size = terrain.size();
	tiles.resize(size.x * size.y);
	for (int y = 0; y < size.y; ++y) {
		for (int x = 0; x < size.x; ++x) {
			auto val = terrain.get(x, y);
			auto &tile = tiles[y * size.x + x];
			switch (val) {
			case 0:
				tile.wall = false;
				tile.groundType = GroundType::Ground;
				break;
			case 1:
				tile.wall = true;
				tile.wallType = WallType::SolidRock;
				break;
			case 2:
				tile.wall = true;
				tile.wallType = WallType::HardRock;
				break;
			case 3:
				tile.wall = true;
				tile.wallType = WallType::LooseRock;
				break;
			case 4:
			case 5:
				tile.wall = true;
				tile.wallType = WallType::Dirt;
				break;
			case 6:
				tile.wall = false;
				tile.groundType = GroundType::Lava;
				break;
			case 8:
				tile.wall = true;
				tile.wallType = WallType::OreSeam;
				break;
			case 9:
				tile.wall = false;
				tile.groundType = GroundType::Water;
				break;
			case 10:
				tile.wall = true;
				tile.wallType = WallType::EnergyCrystalSeam;
				break;
			case 11:
				tile.wall = true;
				tile.wallType = WallType::RechargeSeam;
				break;
			}
		}
	}

	auto crorPath = config.get(levelBlock + "CryoreMap");
	auto duggPath = config.get(levelBlock + "PredugMap");
	auto highPath = config.get(levelBlock + "SurfaceMap");
	auto tutoPath = config.get(levelBlock + "BlockPointersMap");

	setup(archive);
}

void Level::setup(WadArchive &archive) {
	// Setup texture atlas
	atlas.create(2048, 2048, nullptr);

	// Ugly hack for roof textures that are only 64x64 instead of 128x128
	if (roofTexture == "World\\WorldTextures\\lavaroof.bmp" || roofTexture == "World\\WorldTextures\\rockroof.bmp") {
		//atlas.add(archive, roofTexture, glm::ivec2(0, 0), glm::ivec2(128, 128));
		roofUnit = 0.03125f;
	} else {
		//atlas.add(archive, roofTexture, glm::ivec2(0, 0));
		roofUnit = 0.06215f;
	}
	atlas.add(archive, roofTexture, glm::ivec2(0, 0));

	atlas.add(archive, textureBaseName + "00.bmp", glm::ivec2(128, 0));
	atlas.add(archive, textureBaseName + "01.bmp", glm::ivec2(256, 0));
	atlas.add(archive, textureBaseName + "02.bmp", glm::ivec2(384, 0));
	atlas.add(archive, textureBaseName + "03.bmp", glm::ivec2(512, 0));
	atlas.add(archive, textureBaseName + "04.bmp", glm::ivec2(640, 0));
	atlas.add(archive, textureBaseName + "05.bmp", glm::ivec2(768, 0));
	if (roofTexture != "World\\WorldTextures\\iceroof.bmp") {
		atlas.add(archive, textureBaseName + "06.bmp", glm::ivec2(896, 0));
	}
	// 07 1024
	// 08 1152
	// 09 1280
	atlas.add(archive, textureBaseName + "10.bmp", glm::ivec2(1408, 0));
	atlas.add(archive, textureBaseName + "11.bmp", glm::ivec2(1536, 0));
	atlas.add(archive, textureBaseName + "12.bmp", glm::ivec2(1664, 0));
	atlas.add(archive, textureBaseName + "13.bmp", glm::ivec2(1792, 0));
	// 14 1920
	// 15 0
	if (roofTexture != "World\\WorldTextures\\iceroof.bmp") {
		atlas.add(archive, textureBaseName + "16.bmp", glm::ivec2(128, 128));
	}
	// 17 256
	// 18 384
	// 19 512
	atlas.add(archive, textureBaseName + "20.bmp", glm::ivec2(640, 128));
	atlas.add(archive, textureBaseName + "21.bmp", glm::ivec2(768, 128));
	atlas.add(archive, textureBaseName + "22.bmp", glm::ivec2(896, 128));
	atlas.add(archive, textureBaseName + "23.bmp", glm::ivec2(1024, 128));
	atlas.add(archive, textureBaseName + "24.bmp", glm::ivec2(1152, 128));
	atlas.add(archive, textureBaseName + "25.bmp", glm::ivec2(1280, 128));
	if (roofTexture != "World\\WorldTextures\\iceroof.bmp") {
		atlas.add(archive, textureBaseName + "26.bmp", glm::ivec2(1408, 128));
	}
	// 27 1536
	// 28 1664
	// 29 1792
	atlas.add(archive, textureBaseName + "30.bmp", glm::ivec2(1920, 128));
	atlas.add(archive, textureBaseName + "31.bmp", glm::ivec2(0, 256));
	atlas.add(archive, textureBaseName + "32.bmp", glm::ivec2(128, 256));
	atlas.add(archive, textureBaseName + "33.bmp", glm::ivec2(256, 256));
	atlas.add(archive, textureBaseName + "34.bmp", glm::ivec2(384, 256));
	atlas.add(archive, textureBaseName + "35.bmp", glm::ivec2(512, 256));
	if (roofTexture != "World\\WorldTextures\\iceroof.bmp") {
		atlas.add(archive, textureBaseName + "36.bmp", glm::ivec2(640, 256));
	}
	// 37 768
	// 38 896
	// 39 1024
	atlas.add(archive, textureBaseName + "40.bmp", glm::ivec2(1152, 256));
	// 41 1280
	// 42 1408
	// 43 1536
	// 44 1664
	atlas.add(archive, textureBaseName + "45.bmp", glm::ivec2(1792, 256));
	atlas.add(archive, textureBaseName + "46.bmp", glm::ivec2(1920, 256));
	// 47 0
	// 48 128
	// 49 256
	// 50 384
	atlas.add(archive, textureBaseName + "51.bmp", glm::ivec2(512, 384));
	atlas.add(archive, textureBaseName + "52.bmp", glm::ivec2(640, 384));
	atlas.add(archive, textureBaseName + "53.bmp", glm::ivec2(768, 384));
	atlas.add(archive, textureBaseName + "54.bmp", glm::ivec2(896, 384));
	atlas.add(archive, textureBaseName + "55.bmp", glm::ivec2(1024, 384));
	// 56 1152
	// 57 1280
	// 58 1408
	// 59 1536
	atlas.add(archive, textureBaseName + "60.bmp", glm::ivec2(1664, 384));
	atlas.add(archive, textureBaseName + "61.bmp", glm::ivec2(1792, 384));
	atlas.add(archive, textureBaseName + "62.bmp", glm::ivec2(1920, 384));
	atlas.add(archive, textureBaseName + "63.bmp", glm::ivec2(0, 512));
	atlas.add(archive, textureBaseName + "64.bmp", glm::ivec2(128, 512));
	atlas.add(archive, textureBaseName + "65.bmp", glm::ivec2(256, 512));
	atlas.add(archive, textureBaseName + "66.bmp", glm::ivec2(384, 512));
	atlas.add(archive, textureBaseName + "67.bmp", glm::ivec2(512, 512));
	// 68 640
	// 69 768
	atlas.add(archive, textureBaseName + "70.bmp", glm::ivec2(896, 512));
	atlas.add(archive, textureBaseName + "71.bmp", glm::ivec2(1024, 512));
	atlas.add(archive, textureBaseName + "72.bmp", glm::ivec2(1152, 512));
	atlas.add(archive, textureBaseName + "73.bmp", glm::ivec2(1280, 512));
	atlas.add(archive, textureBaseName + "74.bmp", glm::ivec2(1408, 512));
	atlas.add(archive, textureBaseName + "75.bmp", glm::ivec2(1536, 512));
	atlas.add(archive, textureBaseName + "76.bmp", glm::ivec2(1664, 512));
	atlas.add(archive, textureBaseName + "77.bmp", glm::ivec2(1792, 512));

	vertices.resize(size.x * size.y * 6);
	for (int y = 0; y < size.y; ++y) {
		for (int x = 0; x < size.x; ++x) {
			auto &t = tiles[y * size.x + x];
			int walls = 0;
			int i = 0;
			for (int yy = -1; yy < 2; ++yy) {
				for (int xx = -1; xx < 2; ++xx) {
					auto *neighbor = tile(x + xx, y + yy);
					if (neighbor == nullptr || neighbor->wall) {
						walls |= (1 << i);
					}
					++i;
				}
			}
			if (t.wall) {
				if ((bitCount(walls) < 3)) {
					t.wall = false;
					t.groundType = GroundType::Ground;
					t.rubble = 1.0f;
				} else {
					auto index = (y * size.x + x) * 6;
					vertices[index + 0].position = glm::vec3(x, 1, y) * blockSize;
					vertices[index + 1].position = glm::vec3(x + 1, 1, y) * blockSize;
					vertices[index + 2].position = glm::vec3(x + 1, 1, y - 1) * blockSize;
					vertices[index + 3].position = glm::vec3(x + 1, 1, y - 1) * blockSize;
					vertices[index + 4].position = glm::vec3(x, 1, y - 1) * blockSize;
					vertices[index + 5].position = glm::vec3(x, 1, y) * blockSize;

					setUVs(&vertices[index], 0);
					/*vertices[index + 0].uv = glm::vec2(0.0f, 0.0f);
					vertices[index + 1].uv = glm::vec2(0.0625f, 0.0f);
					vertices[index + 2].uv = glm::vec2(0.0625f, 0.0625f);
					vertices[index + 3].uv = glm::vec2(0.0625f, 0.0625f);
					vertices[index + 4].uv = glm::vec2(0.0f, 0.0625f);
					vertices[index + 5].uv = glm::vec2(0.0f, 0.0f);*/
				}
			}
			if (!t.wall) {
				auto index = (y * size.x + x) * 6;
				vertices[index + 0].position = glm::vec3(x, 0, y) * blockSize;
				vertices[index + 1].position = glm::vec3(x + 1, 0, y) * blockSize;
				vertices[index + 2].position = glm::vec3(x + 1, 0, y - 1) * blockSize;
				vertices[index + 3].position = glm::vec3(x + 1, 0, y - 1) * blockSize;
				vertices[index + 4].position = glm::vec3(x, 0, y - 1) * blockSize;
				vertices[index + 5].position = glm::vec3(x, 0, y) * blockSize;

				setUVs(&vertices[index], 1);
				/*vertices[index + 0].uv = glm::vec2(0.0f, 0.0f);
				vertices[index + 1].uv = glm::vec2(0.0625f, 0.0f);
				vertices[index + 2].uv = glm::vec2(0.0625f, 0.0625f);
				vertices[index + 3].uv = glm::vec2(0.0625f, 0.0625f);
				vertices[index + 4].uv = glm::vec2(0.0f, 0.0625f);
				vertices[index + 5].uv = glm::vec2(0.0f, 0.0f);*/
			}
		}
	}

	buffer.upload(vertices.data(), vertices.size() * sizeof(LevelVertex) / sizeof(float));

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	buffer.bind();
	auto stride = 9;
	buffer.setAttribute(0, 3, stride, 0);
	buffer.setAttribute(1, 2, stride, 3);
	buffer.setAttribute(2, 4, stride, 5);
	glBindVertexArray(0);
}