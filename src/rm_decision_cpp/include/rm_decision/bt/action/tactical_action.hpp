#pragma once

#include <behaviortree_cpp/action_node.h>

#include <string>

namespace rm_decision
{

class SetTacticalMode : public BT::SyncActionNode
{
public:
  SetTacticalMode(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

class ChangeTacticalAction : public BT::SyncActionNode
{
public:
  ChangeTacticalAction(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

}  // namespace rm_decision
