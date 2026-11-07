#include "rm_decision/bt/action/gimbal_action.hpp"
#include "rm_decision/bt/bt_utils.hpp"
#include "rm_decision/domain/blackboard_keys.hpp"

#include <tf2/exceptions.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <utility>

namespace rm_decision
{
namespace
{
double nowSec()
{
    const auto now = std::chrono::steady_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::duration<double>>(now).count();
}
} // namespace

TrackTargetAction::TrackTargetAction(
    const std::string & n, const BT::NodeConfig & c, rclcpp::Node::SharedPtr node)
    : StatefulActionNode(n, c), node_(std::move(node)), tf_buffer_(node_->get_clock()),
      tf_listener_(tf_buffer_, node_, false)
{
}
BT::PortsList TrackTargetAction::providedPorts()
{
    return {BT::InputPort<double>("standoff", 0.0, ""),
            BT::InputPort<double>("keep_dist", 0.0, "")};
}
BT::NodeStatus TrackTargetAction::onStart()
{
    // TODO: 自瞄追击链路。
    // 云台在追击期间由自瞄自己闭环，决策只在观测时用 SetGimbalPose 发固定角度。
    // 本节点把自瞄给的目标位置换算成 map 系航点，交给导航去追击。
    // 订阅 /tracker/target（decision_interfaces/msg/Target，SensorData QoS）：
    //   header.frame_id = "odom"（rm_tracker 的 target_frame 参数）
    //   tracking       bool      是否有目标
    //   position       Point     目标中心，【odom 系】，单位 m  ← 直接可用
    //   velocity       Vector3   odom 系速度
    //   id             string    "one".."five" / "sentry" / "base" / "outpost"
    //   yaw / v_yaw / radius / armors_num / visiable_num / dr1-3 / dz1-3
    // 该消息没有 distance 字段，距离要自己算。
    // 要做的事
    //   1. AutoAimIngress 订阅 /tracker/target，写 kTargetValid = tracking，
    //      并把 position 转成 map 系后写 kTargetPoseX / kTargetPoseY；
    //      超时（如 0.2s 无新消息）时 kTargetValid = false。
    //   2. 坐标系：自瞄是 odom，nav2 的 global_frame 是 map（pb2025_nav_bringup/config），
    //      本包 poseFromXyYaw 也写 frame_id = "map"。所以只需 map <- odom 一次变换，
    //      用 tf2_ros::Buffer::transform() 即可（tf2_ros 在 jazzy 里有，但本包
    //      CMakeLists/package.xml 目前只依赖 tf2，需要补 tf2_ros）。
    //      注意：不需要「车体系角 -> map」的反算，也不需要 OdomPoseIngress 提供本车位姿。
    //   3. 距离：kTargetDistance = hypot(目标 map 坐标 - 本车 map 坐标)，
    //      本车坐标可用 tf2 查 map <- base_link；若只做距离门限也可直接用 odom 系差值。
    //   4. 航点：沿「本车 -> 目标」方向回退 standoff 距离，写入 kNavGoal（poseFromXyYaw）
    //      并置 kNavGoalValid = true，由 NavigateToPoseAction 执行。
    //   5. 距离门限：kTargetDistance <= keep_dist 时置 kNavGoalValid = false 停车，
    //      只让自瞄转云台，避免顶到目标。
    //   6. 目标丢失/超时 -> 返回 FAILURE，让树回落到扫描分支。
    // onRunning() 里按目标移动重算航点。
    //
    // 注意：本节点不写 kGimbalYawCmd / kGimbalPitchCmd / kGimbalEnabled；
    // 追击时云台归自瞄，若同时写会两边抢控制权导致云台抖动。
    return updateGoal_();
}
BT::NodeStatus TrackTargetAction::onRunning()
{
    return updateGoal_();
}
void TrackTargetAction::onHalted()
{
    config().blackboard->set(BbKey::kNavGoalValid, false);
}
BT::NodeStatus TrackTargetAction::updateGoal_()
{
    (void)config().blackboard->set(BbKey::kNavGoalValid, false);
    
    bool target_valid = false;
    double target_x = 0.0, target_y = 0.0, distance = 0.0;
    (void)config().blackboard->get(BbKey::kTargetValid, target_valid);
    (void)config().blackboard->get(BbKey::kTargetPoseX, target_x);
    (void)config().blackboard->get(BbKey::kTargetPoseY, target_y);
    (void)config().blackboard->get(BbKey::kTargetDistance, distance);
    double standoff = 0.0, keep_dist = 0.0;
    (void)getInput("standoff", standoff);
    (void)getInput("keep_dist", keep_dist);
    if (distance <= keep_dist) {
        return BT::NodeStatus::RUNNING;
    }

    geometry_msgs::msg::TransformStamped transform;
    try {
        transform = tf_buffer_.lookupTransform("map", "base_link", tf2::TimePointZero);
    } catch (const tf2::TransformException &) {
        return BT::NodeStatus::FAILURE;
    }
    double current_x = transform.transform.translation.x;
    double current_y = transform.transform.translation.y;
    double dx = target_x - current_x;
    double dy = target_y - current_y;
    double scale = std::max(0.0, distance - standoff) / distance;
    double goal_x = current_x + scale * dx;
    double goal_y = current_y + scale * dy;
    (void)config().blackboard->set(BbKey::kNavGoal, poseFromXyYaw(goal_x, goal_y, std::atan2(dy, dx)));
    (void)config().blackboard->set(BbKey::kNavGoalValid, true);
    return BT::NodeStatus::RUNNING;
}

SetGimbalPose::SetGimbalPose(const std::string & n, const BT::NodeConfig & c)
    : SyncActionNode(n, c)
{
}
BT::PortsList SetGimbalPose::providedPorts()
{
    return {
        BT::InputPort<float>("yaw", 0.f, "目标云台 yaw"),
        BT::InputPort<float>("pitch", 0.f, "目标云台 pitch"),
        BT::InputPort<bool>("enable", true, "true=接管云台；false=交还控制权，yaw/pitch 不生效"),
    };
}
BT::NodeStatus SetGimbalPose::tick()
{
    float yaw = 0, pitch = 0;
    bool enable = true;
    (void)getInput("yaw", yaw);
    (void)getInput("pitch", pitch);
    (void)getInput("enable", enable);
    config().blackboard->set(BbKey::kGimbalYawCmd, yaw);
    config().blackboard->set(BbKey::kGimbalPitchCmd, pitch);
    config().blackboard->set(BbKey::kGimbalEnabled, enable);
    return BT::NodeStatus::SUCCESS;
}

SetEnergyAim::SetEnergyAim(const std::string & n, const BT::NodeConfig & c) : SyncActionNode(n, c)
{
}
BT::PortsList SetEnergyAim::providedPorts()
{
    return {
        BT::InputPort<int>("aim_mode", 2, ""), 
        BT::InputPort<double>("hold_sec", 15.0, ""),
        BT::InputPort<bool>("include_active", false, "")};
}
BT::NodeStatus SetEnergyAim::tick()
{
    bool activatable = false;
    int big_status = 0;
    (void)config().blackboard->get(BbKey::kEnergyActivatable, activatable);
    (void)config().blackboard->get(BbKey::kBigEnergyStatus, big_status);
    bool include_active = false;
    (void)getInput("include_active", include_active);
    const bool active = (include_active && activatable) || big_status != 0;

    int aim_mode = 2;
    (void)getInput("aim_mode", aim_mode);
    if (aim_mode != 1 && aim_mode != 2) {
        aim_mode = 2;
    }
    double hold_sec = 15.0;
    (void)getInput("hold_sec", hold_sec);

    std::uint8_t mode = 0;
    if (!active) {
        window_open_ = false;
        start_sec_ = -1.0;
    } else {
        if (!window_open_) {
            window_open_ = true;
            start_sec_ = nowSec();
        }
        const bool within_hold = hold_sec <= 0.0 || (nowSec() - start_sec_) < hold_sec;
        if (within_hold) {
            mode = static_cast<std::uint8_t>(aim_mode);
        }
    }
    config().blackboard->set(BbKey::kEnergyAimMode, mode);
    return BT::NodeStatus::SUCCESS;
}

} // namespace rm_decision
