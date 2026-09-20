#pragma once

#include "rm_decision/domain/zone_map.hpp"

#include <behaviortree_cpp/condition_node.h>

#include <string>

namespace rm_decision
{

class CheckHeat : public BT::ConditionNode
{
public:
  CheckHeat(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

/// P0 XML alias.
using CheckHeatHigh = CheckHeat;

class CheckEngagedStatus : public BT::ConditionNode
{
public:
  CheckEngagedStatus(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

/// P0 XML alias.
using CheckEngaged = CheckEngagedStatus;

class CheckOutpostTarget : public BT::ConditionNode
{
public:
  CheckOutpostTarget(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

class CheckOutpostLowHealthDefend : public BT::ConditionNode
{
public:
  CheckOutpostLowHealthDefend(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

class CheckEnemyAreaRecentlyHurt : public BT::ConditionNode
{
public:
  CheckEnemyAreaRecentlyHurt(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

class CheckTargetDistance : public BT::ConditionNode
{
public:
  CheckTargetDistance(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

class CheckCrossZoneTransition : public BT::ConditionNode
{
public:
  CheckCrossZoneTransition(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts() { return {}; }
  BT::NodeStatus tick() override;
};

class CheckCapacitorCapacity : public BT::ConditionNode
{
public:
  CheckCapacitorCapacity(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

class CheckStanceCooldown : public BT::ConditionNode
{
public:
  CheckStanceCooldown(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

class CheckStanceEffectLimit : public BT::ConditionNode
{
public:
  CheckStanceEffectLimit(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

class CheckStanceRefreshRequired : public BT::ConditionNode
{
public:
  CheckStanceRefreshRequired(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

class CheckTunnelDeformation : public BT::ConditionNode
{
public:
  CheckTunnelDeformation(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts() { return {}; }
  BT::NodeStatus tick() override;
};

class CheckInEnemyFortZone : public BT::ConditionNode
{
public:
  CheckInEnemyFortZone(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts() { return {}; }
  BT::NodeStatus tick() override;
};

class CheckEnemyDefenseHealthDrop : public BT::ConditionNode
{
public:
  CheckEnemyDefenseHealthDrop(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

class CheckManualStanceOverride : public BT::ConditionNode
{
public:
  CheckManualStanceOverride(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts() { return {}; }
  BT::NodeStatus tick() override;
};

class CheckShouldEnhanceStance : public BT::ConditionNode
{
public:
  CheckShouldEnhanceStance(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

}  // namespace rm_decision
