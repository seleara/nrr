#include <nrr/units/unitinfo.h>

std::map<std::string, std::shared_ptr<UnitInfo>, std::less<>> UnitCompendium::unitInfos_;

WadArchive *UnitFactory::archive_;