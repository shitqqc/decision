#pragma once

#include <behaviortree_cpp/blackboard.h>
#include <geometry_msgs/msg/pose.hpp>

namespace rm_decision
{

class INavInfoIngress
{
public:
  virtual ~INavInfoIngress() = default;

  virtual void update(BT::Blackboard & _bb) = 0;
};

} // namespace rm_decision
