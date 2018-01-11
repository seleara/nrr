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
	static void add(ConfigParser &legoCfg, WadArchive &archive, std::string_view unitName, UnitType unitType);
	static std::shared_ptr<UnitInfo> get(std::string_view unitName);
private:
	static std::string getFolderPath(ConfigParser &legoCfg, std::string_view unitName, UnitType unitType);

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
	static Entity create(EntityManager &em, std::string_view unitName);
private:
	friend class Engine;

	static WadArchive *archive_;
};