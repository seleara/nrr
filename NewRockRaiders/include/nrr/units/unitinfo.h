#pragma once

#include <vector>

#include <nrr/ecs/ecs.h>
#include <nrr/util/configparser.h>
#include <nrr/math/transform.h>
#include <nrr/model/modelrenderingsystem.h>

enum class UnitType {
	Undefined,
	MiniFigure,
	Vehicle,
	Building,
	Monster,
	Upgrade // Not sure if it should be here
};

struct FoundationSquare {
	enum class Color {
		Green,
		Yellow,
		Blue
	} color;
	int x, y;
};

struct UnitInfo {
	UnitType type;

	std::string name, theName;

	std::string folderPath, aePath;

	int upgradeLevels = 1;
	std::vector<float> routeSpeed;
	float trackDistance;

	std::vector<float> upgradeTime;
	float collRadius, collHeight;
	float pickSphere;

	// Pilot-specific
	std::vector<float> soilDrillTime;
	std::vector<float> looseDrillTime;
	std::vector<float> medDrillTime;
	std::vector<float> hardDrillTime;
	std::vector<float> seamDrillTime;

	// Building-specific
	std::vector<FoundationSquare> shape;
};

class UnitCompendium {
public:
	static void add(ConfigParser &legoCfg, WadArchive &archive, std::string_view unitName, UnitType unitType) {
		auto info = std::make_shared<UnitInfo>();

		info->type = unitType;
		
		// Names
		info->name = legoCfg.get("Lego*/ObjectNames/" + unitName);
		info->name = legoCfg.get("Lego*/ObjectTheNames/" + unitName);

		switch (info->type) {
		case UnitType::MiniFigure:
			info->folderPath = legoCfg.get("Lego*/MiniFigureTypes/" + unitName);
			break;
		case UnitType::Vehicle:
			info->folderPath = legoCfg.get("Lego*/VehicleTypes/" + unitName);
			break;
		case UnitType::Building:
			info->folderPath = legoCfg.get("Lego*/BuildingTypes/" + unitName);
			break;
		case UnitType::Monster:
			info->folderPath = legoCfg.get("Lego*/RockMonsterTypes/" + unitName);
			break;
		case UnitType::Upgrade:
			info->folderPath = legoCfg.get("Lego*/UpgradeTypes/" + unitName);
			break;
		}

		std::replace(info->folderPath.begin(), info->folderPath.end(), '\\', '/');
		auto aeName = info->folderPath.substr(info->folderPath.find_last_of('/') + 1);
		info->aePath = info->folderPath + '/' + aeName + ".ae";

		if (info->type == UnitType::Building) {
			ConfigParser ae;
			ae.parse(archive, info->aePath);
			std::stringstream ss;
			if (ae.exists("Lego*/Shape")) {
				auto shape = ae.get("Lego*/Shape");
				auto squares = StringUtil::splitRef(shape, { ':' });

				auto strToNum = [&ss](std::string_view &sqstr)->glm::ivec2 {
					auto strstr = std::string(sqstr);
					auto coords = StringUtil::splitRef(strstr, { ',' });
					glm::ivec2 square;
					ss.str("");
					ss.clear();
					ss << std::string(coords[0]) << " " << std::string(coords[1]);
					std::cout << ss.str() << "\n";
					//ss >> square.y >> square.x; -- According to tutorial from RRU
					ss >> square.x >> square.y; // Seems more accurate
					square.y = -square.y;
					return square;
				};

				glm::ivec2 lastSquare = glm::ivec2(0, 0);
				auto lastSquareCol = FoundationSquare::Color::Green;
				int start = 0;
				for (int i = start; i < squares.size(); ++i) {
					auto sqstr = squares[i];
					auto square = strToNum(sqstr);
					if (square == lastSquare) {
						lastSquareCol = FoundationSquare::Color::Yellow;
					} else {
						info->shape.push_back(FoundationSquare{ lastSquareCol, lastSquare.x, lastSquare.y });
						lastSquareCol = FoundationSquare::Color::Green;
					}
					if (i == squares.size() - 1) {
						info->shape.push_back(FoundationSquare{ lastSquareCol, square.x, square.y });
					}
					lastSquare = square;
				}
			}
		}

		unitInfos_.insert({ std::string(unitName), info });
	}

	static std::shared_ptr<UnitInfo> get(std::string_view unitName) {
		auto iter = unitInfos_.find(unitName);
		if (iter == unitInfos_.end()) {
			return nullptr;
		}
		return iter->second;
	}
private:
	static std::map<std::string, std::shared_ptr<UnitInfo>, std::less<>> unitInfos_;
};

struct UnitComponent : public Component<UnitComponent> {
	UnitComponent(const std::string &unitName) : info_(UnitCompendium::get(unitName)) {}
	UnitComponent(UnitInfo &info) : info_(info_) {}
	UnitComponent(const UnitComponent &other) : info_(other.info_) {}
	UnitComponent(UnitComponent &&other) : info_(std::move(other).info_) {}
	UnitComponent &operator=(const UnitComponent &other) {
		info_ = other.info_;
	}
	const UnitInfo &info() const {
		return *info_;
	}
private:
	std::shared_ptr<UnitInfo> info_;
};

class UnitFactory {
public:
	static Entity create(EntityManager &em, std::string_view unitName) {
		auto info = UnitCompendium::get(unitName);
		
		auto unit = em.create();
		unit.add<TransformComponent>();
		auto model = unit.add<ModelComponent>();
		model->load(*archive_, info->aePath);

		return unit;
	}
private:
	friend class Engine;

	static WadArchive *archive_;
};