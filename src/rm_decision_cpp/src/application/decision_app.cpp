#include "rm_decision/application/decision_app.hpp"

#include "rm_decision/domain/blackboard_keys.hpp"
#include "rm_decision/domain/zone_loader.hpp"
#include "rm_decision/domain/zone_update.hpp"
#include "rm_decision/infrastructure/bt_snapshot_publisher.hpp"
#include "rm_decision/infrastructure/game_info_referee_ingress.hpp"
#include "rm_decision/infrastructure/odom_pose_ingress.hpp"
#include "rm_decision/infrastructure/sentry_cmd_egress.hpp"

#include <ament_index_cpp/get_package_share_directory.hpp>
#include <rcutils/logging.h>

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <thread>

namespace rm_decision
{

namespace
{
std::string resolveTopic(rclcpp::Node & node, const std::string & key, const std::string & def)
{
  std::string topic = node.get_parameter_or<std::string>(key, def);
  if (!topic.empty() && topic.front() != '/') {
    const std::string ns = node.get_namespace();
    if (!ns.empty() && ns != "/") {
      topic = ns + "/" + topic;
    }
  }
  return topic;
}
}  // namespace


DecisionApp::DecisionApp(rclcpp::Node::SharedPtr _node)
: node_(std::move(_node))
{
}


void DecisionApp::initDefaults_(BT::Blackboard & _bb)
{
  _bb.set(BbKey::kNavGoalValid, false);
  _bb.set(BbKey::kPoseValid, false);
  _bb.set(BbKey::kTacticalMode, static_cast<std::uint8_t>(TacticalMode_e::Normal));
  _bb.set(BbKey::kNavMode, static_cast<std::uint8_t>(NavMode_e::Patrol));
  _bb.set(BbKey::kDesiredStance, static_cast<std::uint8_t>(StanceCmd_e::Move));
  _bb.set(BbKey::kUseSpin, false);
  _bb.set(BbKey::kUseSuperCap, false);
  _bb.set(BbKey::kReviveRequest, false);
  _bb.set(BbKey::kAmmoPurchaseTotal, static_cast<std::uint16_t>(0));
  _bb.set(BbKey::kBuyHpTimes, static_cast<std::uint16_t>(0));
  _bb.set(BbKey::kBuyProjectileTimes, static_cast<std::uint16_t>(0));
  _bb.set(BbKey::kInEnemyFortZone, false);
  _bb.set(BbKey::kInOwnSupplyZone, false);
  _bb.set(BbKey::kInOwnOutpostZone, false);
  _bb.set(BbKey::kCurrentStance, static_cast<std::uint8_t>(StanceCmd_e::Move));
  _bb.set(BbKey::kEnemyOutpostDestroyed, false);
  _bb.set(BbKey::kOutpostAttackState, 0);
  _bb.set(BbKey::kHeroGuardActive, false);
  _bb.set(BbKey::kHighlandFallbackActive, false);
  _bb.set(BbKey::kTargetValid, false);
  _bb.set(BbKey::kCapacitorCapacity, 1.0f);
  _bb.set(BbKey::kManualStanceOverrideActive, false);
  _bb.set(BbKey::kThroughTunnel, false);
  _bb.set(BbKey::kCurrentInTunnel, false);
  _bb.set(BbKey::kTunnelEscapeActive, false);
  _bb.set(BbKey::kTunnelAlignActive, false);
  _bb.set(BbKey::kOutpostAutoAttackActive, false);
  _bb.set(BbKey::kOutpostManualAttackActive, false);
  _bb.set(BbKey::kOutpostRetreatActive, false);
  _bb.set(BbKey::kOutpostEnhancedDefendActive, false);
  _bb.set(BbKey::kPatrolIndex, 0);
  _bb.set(BbKey::kPatrolBranch, 0);
}


bool DecisionApp::start()
{
  const std::string log_level = node_->get_parameter_or<std::string>("log_level", "info");
  int severity = RCUTILS_LOG_SEVERITY_INFO;
  if (log_level == "debug") {
    severity = RCUTILS_LOG_SEVERITY_DEBUG;
  } else if (log_level == "warn" || log_level == "warning") {
    severity = RCUTILS_LOG_SEVERITY_WARN;
  } else if (log_level == "error") {
    severity = RCUTILS_LOG_SEVERITY_ERROR;
  }
  const auto ret = rcutils_logging_set_logger_level(node_->get_logger().get_name(), severity);
  if (ret != RCUTILS_RET_OK) {
    RCLCPP_WARN(node_->get_logger(), "Failed to set log level");
  }

  std::string zones_path = node_->get_parameter_or<std::string>("zones_yaml", "");
  if (zones_path.empty()) {
    try {
      zones_path =
        ament_index_cpp::get_package_share_directory("rm_decision_cpp") + "/config/zones.yaml";
    } catch (const std::exception &) {
      zones_path.clear();
    }
  }
  std::string zone_err;
  if (zones_path.empty() || !loadZoneMapFromYaml(zones_path, &zones_, &zone_err)) {
    loadDefaultRmucZones(&zones_);
    RCLCPP_WARN(
      node_->get_logger(), "zones yaml fallback to RMUC defaults (%s)",
      zone_err.empty() ? "no path" : zone_err.c_str());
  } else {
    RCLCPP_INFO(node_->get_logger(), "Loaded zones from %s", zones_path.c_str());
  }

  const std::string game_info_topic = resolveTopic(*node_, "game_info_topic_name", "game_info");
  referee_ = std::make_unique<GameInfoRefereeIngress>(*node_, game_info_topic);

  const std::string odom_topic =
    node_->get_parameter_or<std::string>("odom_topic", "/odom");
  pose_ = std::make_unique<OdomPoseIngress>(*node_, odom_topic);

  const std::string cmd_topic = resolveTopic(*node_, "sentry_cmd_topic", "sentry_cmd");
  const std::string spin_topic =
    node_->get_parameter_or<std::string>("use_spin_topic", "/nav/use_spin");
  command_ = std::make_unique<SentryCmdEgress>(*node_, cmd_topic, spin_topic);

  blackboard_ = BT::Blackboard::create();
  initDefaults_(*blackboard_);

  std::string tree_dir;
  try {
    tree_dir = ament_index_cpp::get_package_share_directory("rm_decision_cpp") + "/tree";
  } catch (const std::exception & ex) {
    RCLCPP_ERROR(node_->get_logger(), "package share missing: %s", ex.what());
    return false;
  }

  if (!engine_.initialize(tree_dir, blackboard_, &zones_, node_)) {
    return false;
  }

  enable_bt_monitor_ = node_->get_parameter_or<bool>("enable_bt_monitor", true);
  if (enable_bt_monitor_) {
    const std::string snap_topic =
      resolveTopic(*node_, "bt_snapshot_topic", "bt_snapshot");
    snapshot_ = std::make_unique<BtSnapshotPublisher>(*node_, snap_topic);
  }

  tick_period_ms_ = node_->get_parameter_or<int>("tick_period_milliseconds", 100);
  RCLCPP_INFO(
    node_->get_logger(),
    "DecisionApp started: bit P0, monitor=%s",
    enable_bt_monitor_ ? "on" : "off");
  return true;
}


void DecisionApp::spinLoop()
{
  while (rclcpp::ok()) {
    rclcpp::spin_some(node_);
    referee_->update(*blackboard_);
    pose_->update(*blackboard_);
    updateZoneFlags(zones_, *blackboard_);
    engine_.tickOnce();
    command_->publish(*blackboard_);
    if (snapshot_) {
      snapshot_->publish(
        engine_.resourceTree(), engine_.resourceLatch(), engine_.tacticalTree(),
        engine_.tacticalLatch(), engine_.navTree(), engine_.navLatch(), engine_.stanceTree(),
        engine_.stanceLatch(), engine_.gimbalTree(), engine_.gimbalLatch(), *blackboard_);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(tick_period_ms_));
  }
}

}  // namespace rm_decision
