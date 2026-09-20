#pragma once

#include <behaviortree_cpp/action_node.h>

#include <string>

namespace rm_decision
{

class RequestRevive : public BT::SyncActionNode
{
public:
  RequestRevive(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

/// Alias closer to bit naming.
using RequestReviveAction = RequestRevive;

class ClearRevive : public BT::SyncActionNode
{
public:
  ClearRevive(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts() { return {}; }
  BT::NodeStatus tick() override;
};

class RequestRemoteAmmoExchangeAction : public BT::SyncActionNode
{
public:
  RequestRemoteAmmoExchangeAction(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

class RequestRemoteHealthExchangeAction : public BT::SyncActionNode
{
public:
  RequestRemoteHealthExchangeAction(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

}  // namespace rm_decision
