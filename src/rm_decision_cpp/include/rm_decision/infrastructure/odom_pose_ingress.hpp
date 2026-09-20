#pragma once

#include "rm_decision/interface/i_pose_ingress.hpp"

#include <nav_msgs/msg/odometry.hpp>
#include <rclcpp/rclcpp.hpp>

#include <mutex>
#include <optional>
#include <string>

namespace rm_decision
{

class OdomPoseIngress : public IPoseIngress
{
public:
  OdomPoseIngress(rclcpp::Node & _node, const std::string & _odom_topic);

  void update(BT::Blackboard & _bb) override;

private:
  rclcpp::Node & node_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr sub_;
  mutable std::mutex mutex_;
  std::optional<nav_msgs::msg::Odometry> latest_;
};

}  // namespace rm_decision
