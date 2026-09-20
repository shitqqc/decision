#pragma once

#include "rm_decision/infrastructure/bt_status_latch.hpp"

#include <behaviortree_cpp/bt_factory.h>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>

#include <string>

namespace rm_decision
{

/// Publishes a JSON snapshot of BT trees + blackboard for the web monitor.
class BtSnapshotPublisher
{
public:
  BtSnapshotPublisher(rclcpp::Node & _node, const std::string & _topic);

  void publish(
    const BT::Tree & _resource, const BtStatusLatch & _resource_latch,
    const BT::Tree & _tactical, const BtStatusLatch & _tactical_latch, const BT::Tree & _nav,
    const BtStatusLatch & _nav_latch, const BT::Tree & _stance, const BtStatusLatch & _stance_latch,
    const BT::Tree & _gimbal, const BtStatusLatch & _gimbal_latch, BT::Blackboard & _bb);

private:
  rclcpp::Node & node_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_;
};

}  // namespace rm_decision
