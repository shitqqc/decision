#pragma once

#include <behaviortree_cpp/loggers/abstract_logger.h>

#include <cstdint>
#include <mutex>
#include <unordered_map>

namespace rm_decision
{

/// Remembers each node's last non-IDLE status within the current tick.
/// Reactive* controls reset children to IDLE after SUCCESS/FAILURE; reading
/// TreeNode::status() after tickOnce() therefore looks like everything is IDLE.
class BtStatusLatch : public BT::StatusChangeLogger
{
public:
  explicit BtStatusLatch(BT::TreeNode * _root);

  void beginTick();

  BT::NodeStatus displayStatus(std::uint16_t _uid) const;

private:
  void callback(
    BT::Duration _timestamp, const BT::TreeNode & _node, BT::NodeStatus _prev,
    BT::NodeStatus _status) override;

  void flush() override {}

  mutable std::mutex mutex_;
  std::unordered_map<std::uint16_t, BT::NodeStatus> latched_;
};

}  // namespace rm_decision
