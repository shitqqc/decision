#pragma once

#include <behaviortree_cpp/blackboard.h>

#include <memory>

namespace rm_decision
{

class IRefereeIngress
{
public:
  virtual ~IRefereeIngress() = default;

  virtual void update(BT::Blackboard & _bb) = 0;
};

}  // namespace rm_decision
