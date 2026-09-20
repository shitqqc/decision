#pragma once

#include "rm_decision/domain/zone_map.hpp"

#include <string>

namespace rm_decision
{

/// Load nav points + zones from yaml. On failure, fills RMUC defaults and returns false.
bool loadZoneMapFromYaml(const std::string & _path, ZoneMap * _out, std::string * _err);

/// Hardcoded RMUC defaults (bit area.hpp).
void loadDefaultRmucZones(ZoneMap * _out);

}  // namespace rm_decision
