#pragma once

#include <behaviortree_cpp/action_node.h>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <nav2_msgs/action/navigate_through_poses.hpp>
#include <nav2_msgs/action/navigate_to_pose.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>

#include <memory>
#include <string>
#include <vector>

namespace rm_decision
{

class ZoneMap;

class NavigateToPoseAction : public BT::StatefulActionNode
{
public:
  NavigateToPoseAction(
    const std::string & _name, const BT::NodeConfig & _config, rclcpp::Node::SharedPtr _node,
    const ZoneMap * _zones);
  static BT::PortsList providedPorts();
  BT::NodeStatus onStart() override;
  BT::NodeStatus onRunning() override;
  void onHalted() override;

private:
  bool readGoal_(geometry_msgs::msg::PoseStamped * _out) const;
  bool isDeadInMatch_() const;
  bool readViaPoses_(std::vector<geometry_msgs::msg::PoseStamped> * _out) const;
  void cancelActive_();

  rclcpp::Node::SharedPtr node_;
  const ZoneMap * zones_;
  rclcpp_action::Client<nav2_msgs::action::NavigateToPose>::SharedPtr client_;
  rclcpp_action::Client<nav2_msgs::action::NavigateThroughPoses>::SharedPtr through_client_;
  rclcpp_action::ClientGoalHandle<nav2_msgs::action::NavigateToPose>::SharedPtr goal_handle_;
  rclcpp_action::ClientGoalHandle<nav2_msgs::action::NavigateThroughPoses>::SharedPtr through_handle_;
};

}  // namespace rm_decision
