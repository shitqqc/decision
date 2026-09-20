#pragma once

#include <geometry_msgs/msg/pose_stamped.hpp>
#include <tf2/LinearMath/Quaternion.h>

namespace rm_decision
{

inline geometry_msgs::msg::PoseStamped poseFromXyYaw(double _x, double _y, double _yaw)
{
  geometry_msgs::msg::PoseStamped ps;
  ps.header.frame_id = "map";
  ps.pose.position.x = _x;
  ps.pose.position.y = _y;
  tf2::Quaternion q;
  q.setRPY(0.0, 0.0, _yaw);
  ps.pose.orientation.x = q.x();
  ps.pose.orientation.y = q.y();
  ps.pose.orientation.z = q.z();
  ps.pose.orientation.w = q.w();
  return ps;
}

}  // namespace rm_decision
