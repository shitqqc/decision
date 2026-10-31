#pragma once

#include "rm_decision/interface/i_nav_info_ingress.hpp"

#include <nav_msgs/msg/odometry.hpp>
#include <rclcpp/rclcpp.hpp>

#include <mutex>
#include <optional>
#include <string>

namespace rm_decision
{

// TODO: 类定义整段被注释（见 .cpp 顶部 TODO）。自瞄追击并不必须依赖它
// （目标位置来自 /tracker/target 的 odom 系 position），仅在不想引入 tf2_ros、
// 或需要区域判断时才需要恢复。
// class OdomPoseIngress : public INavInfoIngress
// {
// public:
//   OdomPoseIngress(rclcpp::Node & _node, const std::string & _odom_topic);

//   void update(BT::Blackboard & _bb) override;

// private:
//   rclcpp::Node & node_;
//   rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr sub_;
//   mutable std::mutex mutex_;
//   std::optional<nav_msgs::msg::Odometry> latest_;
// };

} // namespace rm_decision
