#pragma once

#include <behaviortree_cpp/action_node.h>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <nav2_msgs/action/navigate_to_pose.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>

#include <memory>
#include <string>

namespace rm_decision
{

class NavigateToPoseAction : public BT::StatefulActionNode
{
public:
  NavigateToPoseAction(
    const std::string & _name, const BT::NodeConfig & _config, rclcpp::Node::SharedPtr _node);
  static BT::PortsList providedPorts();
  BT::NodeStatus onStart() override;
  BT::NodeStatus onRunning() override;
  void onHalted() override;

private:
  bool readGoal_(geometry_msgs::msg::PoseStamped * _out) const;
  bool isDeadInMatch_() const;

  rclcpp::Node::SharedPtr node_;
  rclcpp_action::Client<nav2_msgs::action::NavigateToPose>::SharedPtr client_;
  rclcpp_action::ClientGoalHandle<nav2_msgs::action::NavigateToPose>::SharedPtr goal_handle_;
};

}  // namespace rm_decision
