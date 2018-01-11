#include <nrr/units/unitinfo.h>

std::map<std::string, std::shared_ptr<UnitInfo>, std::less<>> UnitCompendium::unitInfos_;

WadArchive *UnitFactory::archive_;

void UnitCompendium::add(ConfigParser &legoCfg, WadArchive &archive, std::string_view unitName, UnitType unitType) {
	auto info = std::make_shared<UnitInfo>();

	info->type = unitType;

	// Names
	info->name = legoCfg.get("Lego*/ObjectNames/" + unitName);
	info->theName = legoCfg.get("Lego*/ObjectTheNames/" + unitName);

	info->folderPath = getFolderPath(legoCfg, unitName, info->type);

	std::replace(info->folderPath.begin(), info->folderPath.end(), '\\', '/');
	auto aeName = info->folderPath.substr(info->folderPath.find_last_of('/') + 1);
	info->aePath = info->folderPath + '/' + aeName + ".ae";

	if (info->type == UnitType::Building) {
		ConfigParser ae;
		ae.parse(archive, info->aePath);
		if (ae.exists("Lego*/Shape")) {
			std::stringstream ss;
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
					info->shape.push_back(FoundationSquare { lastSquareCol, lastSquare.x, lastSquare.y });
					lastSquareCol = FoundationSquare::Color::Green;
				}
				if (i == squares.size() - 1) {
					info->shape.push_back(FoundationSquare { lastSquareCol, square.x, square.y });
				}
				lastSquare = square;
			}
		} else {
			// If the building doesn't have a Shape entry, assume it only consists of one green square
			info->shape.push_back(FoundationSquare { FoundationSquare::Color::Green, 0, 0 });
		}
	}

	unitInfos_.insert({ std::string(unitName), info });
}

std::shared_ptr<UnitInfo> UnitCompendium::get(std::string_view unitName) {
	auto iter = unitInfos_.find(unitName);
	if (iter == unitInfos_.end()) {
		return nullptr;
	}
	return iter->second;
}

std::string UnitCompendium::getFolderPath(ConfigParser &legoCfg, std::string_view unitName, UnitType unitType) {
	switch (unitType) {
	case UnitType::MiniFigure:
		return legoCfg.get("Lego*/MiniFigureTypes/" + unitName);
	case UnitType::Vehicle:
		return legoCfg.get("Lego*/VehicleTypes/" + unitName);
	case UnitType::Building:
		return legoCfg.get("Lego*/BuildingTypes/" + unitName);
	case UnitType::Monster:
		return legoCfg.get("Lego*/RockMonsterTypes/" + unitName);
	case UnitType::Upgrade:
		return legoCfg.get("Lego*/UpgradeTypes/" + unitName);
	default:
		return "Undefined path";
	}
}

Entity UnitFactory::create(EntityManager &em, std::string_view unitName) {
	auto info = UnitCompendium::get(unitName);

	auto unit = em.create();
	unit.add<TransformComponent>();
	auto model = unit.add<ModelComponent>();
	model->load(*archive_, info->aePath);

	return unit;
}