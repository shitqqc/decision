#include "rm_decision/bt/action/gimbal_action.hpp"
#include "rm_decision/domain/blackboard_keys.hpp"

#include <chrono>
#include <cstdint>

namespace rm_decision
{
namespace
{
double nowSec()
{
    const auto now = std::chrono::steady_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::duration<double>>(now).count();
}
}  // namespace


TrackTargetAction::TrackTargetAction(const std::string & n, const BT::NodeConfig & c)
    : StatefulActionNode(n, c)
{
}
BT::NodeStatus TrackTargetAction::onStart()
{
    bool v = false;
    (void)config().blackboard->get(BbKey::kTargetValid, v);
    // TODO: gimbal egress not wired
    return v ? BT::NodeStatus::RUNNING : BT::NodeStatus::FAILURE;
}
BT::NodeStatus TrackTargetAction::onRunning()
{
    bool v = false;
    (void)config().blackboard->get(BbKey::kTargetValid, v);
    return v ? BT::NodeStatus::RUNNING : BT::NodeStatus::FAILURE;
}

SetGimbalPose::SetGimbalPose(const std::string & n, const BT::NodeConfig & c) : SyncActionNode(n, c)
{
}
BT::PortsList SetGimbalPose::providedPorts()
{
    return {
        BT::InputPort<float>("yaw", 0.f, ""),
        BT::InputPort<float>("pitch", 0.f, ""),
        BT::InputPort<int>("mode", 1, ""),
    };
}
BT::NodeStatus SetGimbalPose::tick()
{
    float yaw = 0, pitch = 0;
    int mode = 1;
    (void)getInput("yaw", yaw);
    (void)getInput("pitch", pitch);
    (void)getInput("mode", mode);
    config().blackboard->set(BbKey::kGimbalYawCmd, yaw);
    config().blackboard->set(BbKey::kGimbalPitchCmd, pitch);
    config().blackboard->set(BbKey::kPitchMode, mode);
    // TODO: no gimbal hardware egress yet
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
    };
}
BT::NodeStatus SetEnergyAim::tick()
{
    bool activatable = false;
    (void)config().blackboard->get(BbKey::kEnergyActivatable, activatable);

    int aim_mode = 2;
    (void)getInput("aim_mode", aim_mode);
    if (aim_mode != 1 && aim_mode != 2) {
        aim_mode = 2;
    }
    double hold_sec = 15.0;
    (void)getInput("hold_sec", hold_sec);

    std::uint8_t mode = 0;
    if (!activatable) {
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

SetGimbalPoseByAreaAction::SetGimbalPoseByAreaAction(
    const std::string & n, const BT::NodeConfig & c, const ZoneMap * z)
    : SyncActionNode(n, c), zones_(z)
{
}
BT::PortsList SetGimbalPoseByAreaAction::providedPorts()
{
    return {BT::InputPort<std::string>("area", "enemy_outpost", "")};
}
BT::NodeStatus SetGimbalPoseByAreaAction::tick()
{
    (void)zones_;
    config().blackboard->set(BbKey::kPitchMode, 1);
    // TODO: area-based scan limits
    return BT::NodeStatus::SUCCESS;
}

} // namespace rm_decision
