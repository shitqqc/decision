#pragma once

#include "rm_decision/domain/zone_map.hpp"

#include <behaviortree_cpp/action_node.h>

#include <string>

namespace rm_decision
{

class TrackTargetAction : public BT::StatefulActionNode
{
public:
  TrackTargetAction(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts() { return {}; }
  BT::NodeStatus onStart() override;
  BT::NodeStatus onRunning() override;
  void onHalted() override {}
};

class SetGimbalPose : public BT::SyncActionNode
{
public:
  SetGimbalPose(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

class SetGimbalPoseByAreaAction : public BT::SyncActionNode
{
public:
  SetGimbalPoseByAreaAction(
    const std::string & _name, const BT::NodeConfig & _config, const ZoneMap * _zones);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;

private:
  const ZoneMap * zones_;
};

}  // namespace rm_decision
