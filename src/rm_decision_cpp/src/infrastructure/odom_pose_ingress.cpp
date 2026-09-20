#include "rm_decision/infrastructure/odom_pose_ingress.hpp"

#include "rm_decision/domain/blackboard_keys.hpp"

#include <tf2/LinearMath/Matrix3x3.h>
#include <tf2/LinearMath/Quaternion.h>

namespace rm_decision
{

OdomPoseIngress::OdomPoseIngress(rclcpp::Node & _node, const std::string & _odom_topic)
: node_(_node)
{
  sub_ = node_.create_subscription<nav_msgs::msg::Odometry>(
    _odom_topic, rclcpp::SensorDataQoS(),
    [this](const nav_msgs::msg::Odometry::SharedPtr msg) {
      std::lock_guard<std::mutex> lock(mutex_);
      latest_ = *msg;
    });
  RCLCPP_INFO(node_.get_logger(), "OdomPoseIngress: subscribe %s", _odom_topic.c_str());
}


void OdomPoseIngress::update(BT::Blackboard & _bb)
{
  std::optional<nav_msgs::msg::Odometry> copy;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    copy = latest_;
  }
  if (!copy) {
    _bb.set(BbKey::kPoseValid, false);
    return;
  }
  const auto & p = copy->pose.pose;
  tf2::Quaternion q(p.orientation.x, p.orientation.y, p.orientation.z, p.orientation.w);
  double roll = 0, pitch = 0, yaw = 0;
  tf2::Matrix3x3(q).getRPY(roll, pitch, yaw);
  _bb.set(BbKey::kCurrentPoseX, p.position.x);
  _bb.set(BbKey::kCurrentPoseY, p.position.y);
  _bb.set(BbKey::kCurrentPoseYaw, yaw);
  _bb.set(BbKey::kPoseValid, true);
}

}  // namespace rm_decision
