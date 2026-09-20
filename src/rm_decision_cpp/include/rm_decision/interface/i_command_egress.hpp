#pragma once

#include <behaviortree_cpp/blackboard.h>

namespace rm_decision
{

class ICommandEgress
{
public:
  virtual ~ICommandEgress() = default;

  /// Publish SentryCmd / use_spin from blackboard desires.
  virtual void publish(const BT::Blackboard & _bb) = 0;
};

}  // namespace rm_decision
