#pragma once

#include <behaviortree_cpp/action_node.h>

#include <string>

namespace rm_decision
{

class TunnelTimeoutBackoutAction : public BT::StatefulActionNode
{
public:
  TunnelTimeoutBackoutAction(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus onStart() override;
  BT::NodeStatus onRunning() override;
  void onHalted() override {}
};

}  // namespace rm_decision
