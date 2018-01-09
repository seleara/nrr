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
};

class UnitCompendium {
public:
	static void add(ConfigParser &legoCfg, std::string_view unitName, UnitType unitType) {
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