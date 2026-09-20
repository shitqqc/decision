#pragma once

#include "rm_decision/application/bt_engine.hpp"
#include "rm_decision/domain/zone_map.hpp"
#include "rm_decision/infrastructure/bt_snapshot_publisher.hpp"
#include "rm_decision/interface/i_command_egress.hpp"
#include "rm_decision/interface/i_pose_ingress.hpp"
#include "rm_decision/interface/i_referee_ingress.hpp"

#include <rclcpp/rclcpp.hpp>

#include <memory>

namespace rm_decision
{

class DecisionApp
{
public:
  explicit DecisionApp(rclcpp::Node::SharedPtr _node);

  bool start();
  void spinLoop();

private:
  void initDefaults_(BT::Blackboard & _bb);

  rclcpp::Node::SharedPtr node_;
  std::unique_ptr<IRefereeIngress> referee_;
  std::unique_ptr<IPoseIngress> pose_;
  std::unique_ptr<ICommandEgress> command_;
  std::unique_ptr<BtSnapshotPublisher> snapshot_;
  ZoneMap zones_;

  BT::Blackboard::Ptr blackboard_;
  BtEngine engine_;
  int tick_period_ms_{100};
  bool enable_bt_monitor_{true};
};

}  // namespace rm_decision
