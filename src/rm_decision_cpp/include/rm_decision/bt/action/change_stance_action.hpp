#pragma once

#include <behaviortree_cpp/action_node.h>

#include <string>

namespace rm_decision
{

class SetGyroState : public BT::SyncActionNode
{
public:
  SetGyroState(const std::string & _name, const BT::NodeConfig & _config);
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
};

class TunnelGyroAlignAction : public BT::SyncActionNode
{
public:
  TunnelGyroAlignAction(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

class UpdateStanceDuration : public BT::SyncActionNode
{
public:
  UpdateStanceDuration(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts() { return {}; }
  BT::NodeStatus tick() override;
};

class ApplyManualStanceOverride : public BT::SyncActionNode
{
public:
  ApplyManualStanceOverride(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts() { return {}; }
  BT::NodeStatus tick() override;
};

}  // namespace rm_decision
