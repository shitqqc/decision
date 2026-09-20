#include "rm_decision/infrastructure/bt_status_latch.hpp"

namespace rm_decision
{

BtStatusLatch::BtStatusLatch(BT::TreeNode * _root)
: StatusChangeLogger(_root)
{
}


void BtStatusLatch::beginTick()
{
  std::lock_guard<std::mutex> lock(mutex_);
  latched_.clear();
}


BT::NodeStatus BtStatusLatch::displayStatus(std::uint16_t _uid) const
{
  std::lock_guard<std::mutex> lock(mutex_);
  const auto it = latched_.find(_uid);
  if (it == latched_.end()) {
    return BT::NodeStatus::IDLE;
  }
  return it->second;
}


void BtStatusLatch::callback(
  BT::Duration /*_timestamp*/, const BT::TreeNode & _node, BT::NodeStatus /*_prev*/,
  BT::NodeStatus _status)
{
  if (_status == BT::NodeStatus::IDLE) {
    return;
  }
  std::lock_guard<std::mutex> lock(mutex_);
  latched_[_node.UID()] = _status;
}

}  // namespace rm_decision
