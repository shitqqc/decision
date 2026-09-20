#pragma once

#include "rm_decision/domain/blackboard_keys.hpp"
#include "rm_decision/domain/zone_map.hpp"

#include <behaviortree_cpp/blackboard.h>

namespace rm_decision
{

/// Refresh in_* zone flags from current pose + ZoneMap (call each tick before trees).
inline void updateZoneFlags(const ZoneMap & _zones, BT::Blackboard & _bb)
{
  bool pose_valid = false;
  (void)_bb.get(BbKey::kPoseValid, pose_valid);
  double x = 0.0;
  double y = 0.0;
  (void)_bb.get(BbKey::kCurrentPoseX, x);
  (void)_bb.get(BbKey::kCurrentPoseY, y);

  const bool ok = pose_valid;
  _bb.set(BbKey::kInEnemyFortZone, ok && _zones.inZone("enemy_fort", x, y));
  _bb.set(BbKey::kInOwnSupplyZone, ok && _zones.inZone("own_supply", x, y));
  _bb.set(BbKey::kInOwnOutpostZone, ok && _zones.inZone("own_outpost", x, y));
}

}  // namespace rm_decision
