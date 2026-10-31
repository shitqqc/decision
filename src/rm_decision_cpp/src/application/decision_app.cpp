#include "rm_decision/application/decision_app.hpp"

#include "rm_decision/domain/blackboard_keys.hpp"
#include "rm_decision/domain/zone_loader.hpp"
#include "rm_decision/infrastructure/bt_snapshot_publisher.hpp"
#include "rm_decision/infrastructure/game_info_referee_ingress.hpp"
// #include "rm_decision/infrastructure/odom_pose_ingress.hpp"
#include "rm_decision/infrastructure/sentry_cmd_egress.hpp"
#include "rm_decision/infrastructure/through_tunnel_ingress.hpp"

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
} // namespace


DecisionApp::DecisionApp(rclcpp::Node::SharedPtr _node) : node_(std::move(_node)) {}


void DecisionApp::initDefaults_(BT::Blackboard & _bb)
{
    _bb.set(BbKey::kNavGoalValid, false);
    // _bb.set(BbKey::kPoseValid, false);
    _bb.set(BbKey::kTacticalMode, static_cast<std::uint8_t>(TacticalMode_e::Normal));
    _bb.set(BbKey::kNavMode, static_cast<std::uint8_t>(NavMode_e::Patrol));
    _bb.set(BbKey::kDesiredStance, static_cast<std::uint8_t>(StanceCmd_e::Move));
    _bb.set(BbKey::kUseSpin, false);
    _bb.set(BbKey::kUseSuperCap, false);
    _bb.set(BbKey::kReviveRequest, false);
    _bb.set(BbKey::kAmmoPurchaseTotal, static_cast<std::uint16_t>(0));
    _bb.set(BbKey::kBuyHpTimes, static_cast<std::uint16_t>(0));
    _bb.set(BbKey::kBuyProjectileTimes, static_cast<std::uint16_t>(0));
    // _bb.set(BbKey::kInEnemyFortZone, false);
    // _bb.set(BbKey::kInOwnSupplyZone, false);
    // _bb.set(BbKey::kInOwnOutpostZone, false);
    _bb.set(BbKey::kCurrentStance, static_cast<std::uint8_t>(StanceCmd_e::Move));
    _bb.set(BbKey::kEnemyOutpostDestroyed, false);
    _bb.set(BbKey::kHeroGuardActive, false);
    _bb.set(BbKey::kHighlandFallbackActive, false);
    _bb.set(BbKey::kTargetValid, false);
    _bb.set(BbKey::kCapacitorCapacity, 1.0f);
    _bb.set(BbKey::kManualStanceOverrideActive, false);
    _bb.set(BbKey::kOutpostAutoAttackActive, false);
    _bb.set(BbKey::kOutpostManualAttackActive, false);
    _bb.set(BbKey::kOutpostRetreatActive, false);
    _bb.set(BbKey::kOutpostEnhancedDefendActive, false);
    _bb.set(BbKey::kGimbalEnabled, false);
    _bb.set(BbKey::kEnergyAimMode, static_cast<std::uint8_t>(0));
    _bb.set(BbKey::kThroughTunnel, false);
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
            zones_path = ament_index_cpp::get_package_share_directory("rm_decision_cpp") +
                         "/config/zones.yaml";
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

    const std::string game_info_topic = resolveTopic(*node_, "game_info_topic", "game_info");
    referee_ = std::make_unique<GameInfoRefereeIngress>(*node_, game_info_topic);

    // const std::string odom_topic = node_->get_parameter_or<std::string>("odom_topic", "/odom");
    // pose_ = std::make_unique<OdomPoseIngress>(*node_, odom_topic);

    const std::string through_tunnel_topic =
        node_->get_parameter_or<std::string>("through_tunnel_topic", "/through_tunnel");
    tunnel_ = std::make_unique<ThroughTunnelIngress>(*node_, through_tunnel_topic);

    const std::string cmd_topic = resolveTopic(*node_, "sentry_cmd_topic", "sentry_cmd");
    const std::string spin_topic =
        node_->get_parameter_or<std::string>("use_spin_topic", "/nav/use_spin");
    const std::string energy_aim_topic = resolveTopic(*node_, "energy_aim_topic", "energy_aim_mode");
    // TODO: SetGimbalPose 用 "gimbal_pose_topic" 参数（默认 /nav/gimbal_pose）自建 publisher。
    // 等云台输出并入 SentryCmdEgress 后，在这里按上面两个话题的方式解析该参数，并作为第 5 个构造参数传进去。
    command_ = std::make_unique<SentryCmdEgress>(*node_, cmd_topic, spin_topic, energy_aim_topic);

    // TODO: 自瞄追击（见 gimbal_action.cpp 的 TrackTargetAction）需要先把自瞄结果送进 blackboard。
    // 职责边界：决策不做自瞄、也不转发云台角。自瞄自己闭环云台；决策只在追击时用
    // 「自瞄给的目标位置」算 map 系航点交给导航，观测时才由 SetGimbalPose 下发固定角。
    // 数据源（已确认，见 /home/shitqqc/PinyVision）：
    //   订阅 /tracker/target，类型 rm_interfaces/msg/Target，SensorData QoS。
    //   header.frame_id = odom；position 是 odom 系目标中心（m）；tracking 表示有无目标；
    //   velocity 是 odom 系速度；id 为 "one".."five"/"sentry"/"base"/"outpost"。
    //   该消息没有 distance 字段，距离需自己算。（/armor_planner/cmd_gimbal 里的 yaw/pitch 是 odom 系绝对云台角，追击用不上。）
    // 依赖改动：
    //   rm_interfaces 是 PinyVision 工作区里的包（PinyVision/src/rm_interfaces），
    //   给 decision_interfaces 补一个精简的 Target 等价消息，由自瞄侧或桥接节点转发。tf2_ros 也需要补进依赖（当前只有 tf2），用于 map <- odom 变换。
    // 该做的改造：
    //   1. 新建 infrastructure/auto_aim_ingress.{hpp,cpp}，仿照 GameInfoRefereeIngress：
    //      缓存 latest_，update() 里写 kTargetValid <- tracking，
    //      并把 position 经 map<-odom 变换后写 kTargetPoseX / kTargetPoseY，
    //      以及 kTargetDistance <- 目标与本车在 map 系的距离；
    //      超时（例如 0.2s 无新消息）时 kTargetValid 置 false，避免追一个早就消失的目标；
    //   2. 在这里构造该 ingress，并在 spinLoop() 里 referee_->update(...) 之后调用 update；
    //   3. 坐标系：自瞄是 odom，nav2 的 global_frame 是 map，本包 poseFromXyYaw 写的是 "map"，所以只需要 map <- odom 一次变换，不需要从车体系角反算目标位置。

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
        const std::string snap_topic = resolveTopic(*node_, "bt_snapshot_topic", "bt_snapshot");
        snapshot_ = std::make_unique<BtSnapshotPublisher>(*node_, snap_topic);
    }

    tick_period_ms_ = node_->get_parameter_or<int>("tick_period_milliseconds", 100);
    RCLCPP_INFO(
        node_->get_logger(), "DecisionApp started: bit P0, monitor=%s",
        enable_bt_monitor_ ? "on" : "off");
    return true;
}


void DecisionApp::spinLoop()
{
    while (rclcpp::ok()) {
        rclcpp::spin_some(node_);
        referee_->update(*blackboard_);
        // pose_->update(*blackboard_);
        tunnel_->update(*blackboard_);
        // updateZoneFlags(zones_, *blackboard_);
        engine_.tickOnce();
        command_->publish(*blackboard_);
        if (snapshot_) {
            snapshot_->publish(
                engine_.resourceTree(), engine_.resourceLatch(), engine_.tacticalTree(),
                engine_.tacticalLatch(), engine_.navTree(), engine_.navLatch(),
                engine_.stanceTree(), engine_.stanceLatch(), engine_.gimbalTree(),
                engine_.gimbalLatch(), engine_.ulTree(), engine_.ulLatch(), *blackboard_);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(tick_period_ms_));
    }
}

} // namespace rm_decision
