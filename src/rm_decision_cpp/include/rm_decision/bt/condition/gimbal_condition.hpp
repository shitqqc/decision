#pragma once

#include "rm_decision/domain/zone_map.hpp"

#include <behaviortree_cpp/condition_node.h>

#include <string>

namespace rm_decision
{

class CheckTargetVisible : public BT::ConditionNode
{
public:
  CheckTargetVisible(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts() { return {}; }
  BT::NodeStatus tick() override;
};

class CheckNearEnemyOutpost : public BT::ConditionNode
{
public:
  CheckNearEnemyOutpost(
    const std::string & _name, const BT::NodeConfig & _config, const ZoneMap * _zones);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;

private:
  const ZoneMap * zones_;
};

}  // namespace rm_decision
