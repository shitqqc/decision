#pragma once

#include "rm_decision/domain/zone_map.hpp"
#include "rm_decision/infrastructure/bt_status_latch.hpp"

#include <behaviortree_cpp/bt_factory.h>
#include <rclcpp/rclcpp.hpp>

#include <memory>
#include <string>

namespace rm_decision
{

class BtEngine
{
public:
  BtEngine() = default;

  /// Like bit SentryBTManager::initialize: registerNodes then loadTrees.
  bool initialize(
    const std::string & _tree_dir, const BT::Blackboard::Ptr & _blackboard, const ZoneMap * _zones,
    rclcpp::Node::SharedPtr _node);

  /// Tick order: resource → tactical → nav → stance → gimbal
  void tickOnce();

  BT::BehaviorTreeFactory & factory() { return factory_; }

  const BT::Tree & resourceTree() const { return resource_tree_; }
  const BT::Tree & tacticalTree() const { return tactical_tree_; }
  const BT::Tree & navTree() const { return nav_tree_; }
  const BT::Tree & stanceTree() const { return stance_tree_; }
  const BT::Tree & gimbalTree() const { return gimbal_tree_; }

  const BtStatusLatch & resourceLatch() const { return *resource_latch_; }
  const BtStatusLatch & tacticalLatch() const { return *tactical_latch_; }
  const BtStatusLatch & navLatch() const { return *nav_latch_; }
  const BtStatusLatch & stanceLatch() const { return *stance_latch_; }
  const BtStatusLatch & gimbalLatch() const { return *gimbal_latch_; }

private:
  void registerNodes_(const ZoneMap * _zones, rclcpp::Node::SharedPtr _node);
  bool loadTrees_(const std::string & _tree_dir, const BT::Blackboard::Ptr & _blackboard);

  BT::BehaviorTreeFactory factory_;
  BT::Tree resource_tree_;
  BT::Tree tactical_tree_;
  BT::Tree nav_tree_;
  BT::Tree stance_tree_;
  BT::Tree gimbal_tree_;
  std::unique_ptr<BtStatusLatch> resource_latch_;
  std::unique_ptr<BtStatusLatch> tactical_latch_;
  std::unique_ptr<BtStatusLatch> nav_latch_;
  std::unique_ptr<BtStatusLatch> stance_latch_;
  std::unique_ptr<BtStatusLatch> gimbal_latch_;
};

}  // namespace rm_decision
