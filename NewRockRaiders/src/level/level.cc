#include <nrr/level/level.h>

#include <fstream>
#include <sstream>
#include <iomanip>

#include <nrr/level/map.h>
#include <nrr/util/binaryreader.h>
#include <nrr/util/configparser.h>
#include <nrr/math/camera.h>
#include <nrr/math/transform.h>
#include <nrr/units/unitinfo.h>

void Level::load(EntityManager &em, ConfigParser &config, WadArchive &archive, int level) {
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
			std::cout << std::hex << (int)val;
			auto &tile = tiles[(size.y - y - 1) * size.x + x];
			switch (val) {
			case 0:
			case 5:
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
		std::cout << "\n";
	}

	if (config.exists(levelBlock + "CryoreMap")) {
		auto crorPath = config.get(levelBlock + "CryoreMap");
	}

	if (config.exists(levelBlock + "PredugMap")) {
		auto duggPath = config.get(levelBlock + "PredugMap");
		Map predug;
		predug.load(archive, duggPath);
		for (int y = 0; y < size.y; ++y) {
			for (int x = 0; x < size.x; ++x) {
				auto val = predug.get(x, y);
				//std::cout << std::hex << (int)val;
				auto &tile = tiles[(size.y - y - 1) * size.x + x];
				tile.visible = (val == 2 || val == 4 ? false : true);
			}
		}
	}

	if (config.exists(levelBlock + "SurfaceMap")) {
		auto highPath = config.get(levelBlock + "SurfaceMap");
		Map surface;
		surface.load(archive, highPath);
		for (int y = 0; y < size.y; ++y) {
			for (int x = 0; x < size.x; ++x) {
				auto val = surface.get(x, y);
				auto &tile = tiles[(size.y - y - 1) * size.x + x];
				tile.height = (float)val * 20.0f / 6.0f;
			}
		}
	}

	if (config.exists(levelBlock + "BlockPointersMap")) {
		auto tutoPath = config.get(levelBlock + "BlockPointersMap");
	}

	setup(archive);

	if (config.exists(levelBlock + "OListFile")) {
		auto olPath = config.get(levelBlock + "OListFile");
		ConfigParser ol;
		ol.parse(archive, olPath);

		std::stringstream ss;
		for (int i = 1;; ++i) {
			ss << "Lego*/Object" << i;
			auto objPath = ss.str();
			ss.str("");
			if (ol.exists(objPath)) {
				auto type = ol.get(objPath + "/type");
				auto xPos = ol.get<float>(objPath + "/xPos") - 1.0f;
				auto yPos = ol.get<float>(objPath + "/yPos");
				auto heading = ol.get<float>(objPath + "/heading");
				if (type == "TVCamera") {
					auto &pos = CameraComponent::main.entity().get<TransformComponent>()->position;
					pos.x = xPos * blockSize;
					pos.z = yPos * blockSize;
				} else {
					auto unit = UnitFactory::create(em, type);
					auto tc = unit.get<TransformComponent>();
					tc->position = glm::vec3(xPos * blockSize, 60, yPos * blockSize);
					tc->rotation = glm::rotate(glm::quat(), heading / 180.0f * glm::pi<float>(), glm::vec3(0, 1, 0));
					unit.get<ModelComponent>()->play("Activity_Stand");

					auto info = UnitCompendium::get(type);
					int bx = (int)xPos;
					int by = size.y - (int)yPos;
					if (info->type == UnitType::Building) {
						for (auto &sq : info->shape) {
							auto xx = bx + sq.x;
							auto yy = by + sq.y;
							auto t = tile(xx, yy);
							t->pathType = PathType::BuildingPowerPath;
							tile(xx, yy + 1)->height = t->height;
							tile(xx + 1, yy)->height = t->height;
							tile(xx + 1, yy + 1)->height = t->height;
							t->dirty = true;
						}
						auto t = tile(bx, by);
						tc->position.y = t->height;
					} else {
						auto t = tile(bx, by);
						tc->position.y = t->height;
					}
				}
			} else {
				break;
			}
		}
	}
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
	int tx = 0, ty = 0;
	auto nextPos = [&tx, &ty]() {
		glm::ivec2 ret(tx, ty);
		tx += 128;
		if (tx >= 2048) {
			ty += 128;
			tx = 0;
		}
		return ret;
	};

	atlas.add(archive, roofTexture, nextPos());

	bool iceRoof = (roofTexture == "World\\WorldTextures\\iceroof.bmp");

	std::stringstream ss;
	for (int i = 0; i <= 77; ++i) {
		if ((i == 6 || i == 16 || i == 26 || i == 36) && iceRoof) {
			nextPos();
			continue;
		}
		if ((i >= 7 && i <= 9) || (i >= 14 && i <= 15) || (i >= 17 && i <= 19) || (i >= 27 && i <= 29) || (i >= 37 && i <= 39) ||
			(i >= 41 && i <= 44) || (i >= 47 && i <= 50) || (i >= 56 && i <= 59) || (i >= 68 && i <= 69)) {
			nextPos();
			continue;
		}
		ss.str("");
		ss << textureBaseName << std::setfill('0') << std::setw(2) << i << ".bmp";
		atlas.add(archive, ss.str(), nextPos());
	}

	vertices.resize(size.x * size.y * 6);
	for (int y = 0; y < size.y; ++y) {
		for (int x = 0; x < size.x; ++x) {
			updateTile(x, y);
		}
	}

	uploadTiles();

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	buffer.bind();
	auto stride = 9;
	buffer.setAttribute(0, 3, stride, 0);
	buffer.setAttribute(1, 2, stride, 3);
	buffer.setAttribute(2, 4, stride, 5);
	glBindVertexArray(0);
}

bool Level::updateTile(int x, int y) {
	auto &t = tiles[y * size.x + x];
	int walls = 0;
	int i = 0;

	Tile prevT = t;

	// Create a bitmask that tells us which surrounding tiles are walls at a glance
	for (int yy = -1; yy < 2; ++yy) {
		for (int xx = -1; xx < 2; ++xx) {
			if (yy == 0 && xx == 0) continue;
			auto *neighbor = tile(x + xx, y - yy);
			if (neighbor == nullptr || neighbor->wall || !neighbor->visible) {
				walls |= (1 << i);
			}
			++i;
		}
	}
	if (t.wall) {
		int texAdd = (int)t.wallType;
		if ((bitCount(walls) < 3)) {
			t.wallOrientation = WallOrientation::Invalid;
		} else {
			auto index = (y * size.x + x) * 6;
			if (wallMask(walls, "########")) { // All surrounding walls are walls
				t.wallOrientation = WallOrientation::Roof;

				// Edge walls
			} else if (wallMask(walls, "## #. . ")) { // SE edge wall
				t.wallOrientation = WallOrientation::EdgeSE;
			} else if (wallMask(walls, " ##.# . ")) { // SW edge wall
				t.wallOrientation = WallOrientation::EdgeSW;
			} else if (wallMask(walls, " . .# ##")) { // NW edge wall
				t.wallOrientation = WallOrientation::EdgeNW;
			} else if (wallMask(walls, " . #.## ")) { // NE edge wall
				t.wallOrientation = WallOrientation::EdgeNE;

				// Corner walls
			} else if (wallMask(walls, "## ## #.")) { // SE corner wall
				t.wallOrientation = WallOrientation::CornerSE;
			} else if (wallMask(walls, " ####.# ")) { // SW corner wall
				t.wallOrientation = WallOrientation::CornerSW;
			} else if (wallMask(walls, ".# ## ##")) { // NW corner wall
				t.wallOrientation = WallOrientation::CornerNW;
			} else if (wallMask(walls, " #.#### ")) { // NE corner wall
				t.wallOrientation = WallOrientation::CornerNE;

				// Straight walls
			} else if (wallMask(walls, " # ## . ")) { // S wall
				t.wallOrientation = WallOrientation::StraightS;
			} else if (wallMask(walls, " # .# # ")) { // W wall
				t.wallOrientation = WallOrientation::StraightW;
			} else if (wallMask(walls, " . ## # ")) { // N wall
				t.wallOrientation = WallOrientation::StraightN;
			} else if (wallMask(walls, " # #. # ")) { // E wall
				t.wallOrientation = WallOrientation::StraightE;

				// Weird inbetween walls
			} else if (wallMask(walls, ".######.")) { // <0> wall
				t.wallOrientation = WallOrientation::Inbetween0;
			} else if (wallMask(walls, "##.##.##")) { // <1> wall
				t.wallOrientation = WallOrientation::Inbetween1;

				// Unknown
			} else {
				t.wallOrientation = WallOrientation::Invalid;
			}
		}
		if (t.wallOrientation == WallOrientation::Invalid) {
			t.wall = false;
			t.rubble = 1.0f;
			t.groundType = GroundType::Ground;
			t.pathType = PathType::Rubble;
		}
	}
	t.initialized = true;
	if (prevT != t || t.dirty) {
		t.dirty = false;
		updateTileVertices(x, y);
		return true;
	}
	return false;
}

void Level::updateTileVertices(int x, int y) {
	auto &t = *tile(x, y);
	auto index = tileVertexIndex(x, y);

	TileHeights th;
	th.topLeft = t.height;
	if (x < size.x - 1) {
		th.topRight = tile(x + 1, y)->height;
		if (y > 0) {
			th.bottomRight = tile(x + 1, y - 1)->height;
		} else {
			th.bottomRight = th.topRight;
		}
	} else {
		th.topRight = th.topLeft;
		th.bottomRight = th.topLeft;
	}
	if (y > 0) {
		th.bottomLeft = tile(x, y - 1)->height;
	} else {
		th.bottomLeft = th.topLeft;
	}

	if (t.wall) {
		int texAdd = (int)t.wallType;
		if (t.wallOrientation == WallOrientation::Roof) { // All surrounding walls are walls
			setPoints<0>(&vertices[index], x, y, { 1, 1, 1, 1 }, th);
			setUVs<0>(&vertices[index], 0);

			// Edge walls
		} else if (t.wallOrientation == WallOrientation::EdgeSE) { // SE edge wall
			setPoints<0>(&vertices[index], x, y, { 1, 0, 0, 0 }, th);
			setUVs<0>(&vertices[index], 53 + texAdd);
		} else if (t.wallOrientation == WallOrientation::EdgeSW) { // SW edge wall
			setPoints<1>(&vertices[index], x, y, { 0, 1, 0, 0 }, th);
			setUVs<2>(&vertices[index], 53 + texAdd);
		} else if (t.wallOrientation == WallOrientation::EdgeNW) { // NW edge wall
			setPoints<0>(&vertices[index], x, y, { 0, 0, 1, 0 }, th);
			setUVs<2>(&vertices[index], 53 + texAdd);
		} else if (t.wallOrientation == WallOrientation::EdgeNE) { // NE edge wall
			setPoints<1>(&vertices[index], x, y, { 0, 0, 0, 1 }, th);
			setUVs<0>(&vertices[index], 53 + texAdd);

			// Corner walls
		} else if (t.wallOrientation == WallOrientation::CornerSE) { // SE corner wall
			setPoints<0>(&vertices[index], x, y, { 1, 1, 0, 1 }, th);
			setUVs<0>(&vertices[index], 33 + texAdd);
		} else if (t.wallOrientation == WallOrientation::CornerSW) { // SW corner wall
			setPoints<1>(&vertices[index], x, y, { 1, 1, 1, 0 }, th);
			setUVs<2>(&vertices[index], 33 + texAdd);
		} else if (t.wallOrientation == WallOrientation::CornerNW) { // NW corner wall
			setPoints<0>(&vertices[index], x, y, { 0, 1, 1, 1 }, th);
			setUVs<2>(&vertices[index], 33 + texAdd);
		} else if (t.wallOrientation == WallOrientation::CornerNE) { // NE corner wall
			setPoints<1>(&vertices[index], x, y, { 1, 0, 1, 1 }, th);
			setUVs<0>(&vertices[index], 33 + texAdd);

			// Straight walls
		} else if (t.wallOrientation == WallOrientation::StraightS) { // S wall
			setPoints<0>(&vertices[index], x, y, { 1, 1, 0, 0 }, th);
			setUVs<0>(&vertices[index], 3 + texAdd);
		} else if (t.wallOrientation == WallOrientation::StraightW) { // W wall
			setPoints<0>(&vertices[index], x, y, { 0, 1, 1, 0 }, th);
			setUVs<1>(&vertices[index], 3 + texAdd);
		} else if (t.wallOrientation == WallOrientation::StraightN) { // N wall
			setPoints<0>(&vertices[index], x, y, { 0, 0, 1, 1 }, th);
			setUVs<2>(&vertices[index], 3 + texAdd);
		} else if (t.wallOrientation == WallOrientation::StraightE) { // E wall
			setPoints<0>(&vertices[index], x, y, { 1, 0, 0, 1 }, th);
			setUVs<3>(&vertices[index], 3 + texAdd);

			// Weird inbetween walls
		} else if (t.wallOrientation == WallOrientation::Inbetween0) { // <0> wall
			setPoints<0>(&vertices[index], x, y, { 0, 1, 0, 1 }, th);
			setUVs<0>(&vertices[index], 3 + texAdd);
		} else if (t.wallOrientation == WallOrientation::Inbetween1) { // <1> wall
			setPoints<1>(&vertices[index], x, y, { 1, 0, 1, 0 }, th);
			setUVs<0>(&vertices[index], 3 + texAdd);

			// Unknown/Invalid
		} else {
			setPoints<0>(&vertices[index], x, y, { 0, 0, 0, 0 }, th);
			setUVs<0>(&vertices[index], 0);
		}
	}

	if (!t.wall) {
		setPoints<0>(&vertices[index], x, y, { 0, 0, 0, 0 }, th);
		switch (t.groundType) {
		case GroundType::Water:
			setUVs<0>(&vertices[index], 46);
			break;
		case GroundType::Lava:
			setUVs<0>(&vertices[index], 47);
			break;
		case GroundType::SlugHole:
			setUVs<0>(&vertices[index], 31);
			break;
		default:
			switch (t.pathType) {
			case PathType::None:
				setUVs<0>(&vertices[index], 1);
				break;
			case PathType::Rubble:
				setUVs<0>(&vertices[index], 14 - (int)(t.rubble * 3));
				break;
			case PathType::PowerPath:
				setUVs<0>(&vertices[index], 61);
				break;
			case PathType::BuildingPowerPath:
				setUVs<0>(&vertices[index], 77);
				break;
			}
			break;
		}

		if (!t.visible) {
			setColor(&vertices[index], glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
		}
	}

	setColor(&vertices[index], t.highlightColor);
}

void Level::uploadTiles() {
	buffer.upload(vertices.data(), vertices.size() * sizeof(LevelVertex) / sizeof(float));
}

void Level::uploadTile(int x, int y) {
	auto index = tileVertexIndex(x, y);
	buffer.uploadPart(vertices.data() + index, index * sizeof(LevelVertex) / sizeof(float), 6 * sizeof(LevelVertex) / sizeof(float));
}