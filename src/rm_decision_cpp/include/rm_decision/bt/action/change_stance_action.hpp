#pragma once

#include <behaviortree_cpp/action_node.h>

#include <string>

namespace rm_decision
{

class SetSpinState : public BT::SyncActionNode
{
public:
  SetSpinState(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

class SetDesiredStance : public BT::SyncActionNode
{
public:
  SetDesiredStance(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

class SetSuperCap : public BT::SyncActionNode
{
public:
  SetSuperCap(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

class ChangeStance : public BT::StatefulActionNode
{
public:
  ChangeStance(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus onStart() override;
  BT::NodeStatus onRunning() override;
  void onHalted() override {}

private:
  double last_change_sec_{-1.0e9};
};

class UpdateStanceDuration : public BT::SyncActionNode
{
public:
  UpdateStanceDuration(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts() { return {}; }
  BT::NodeStatus tick() override;

private:
  int last_game_time_{-1};
};

class ApplyManualStanceOverride : public BT::SyncActionNode
{
public:
  ApplyManualStanceOverride(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts() { return {}; }
  BT::NodeStatus tick() override;
};

}  // namespace rm_decision
