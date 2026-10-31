#include "rm_decision/infrastructure/sentry_cmd_egress.hpp"

#include "rm_decision/domain/blackboard_keys.hpp"

#include <cstdint>
#include <std_msgs/msg/u_int8.hpp>

namespace rm_decision
{

SentryCmdEgress::SentryCmdEgress(
    rclcpp::Node & _node, const std::string & _sentry_cmd_topic,
    const std::string & _use_spin_topic, const std::string & _energy_aim_topic)
    : node_(_node)
{
    sentry_pub_ = node_.create_publisher<decision_interfaces::msg::SentryCmd>(
        _sentry_cmd_topic, rclcpp::QoS(1).transient_local());
    spin_pub_ = node_.create_publisher<std_msgs::msg::Bool>(_use_spin_topic, rclcpp::QoS(10));
    energy_aim_pub_ =
        node_.create_publisher<std_msgs::msg::UInt8>(_energy_aim_topic, rclcpp::QoS(10));
    // TODO: 新增 _gimbal_pose_topic 参数后，参考上面几个 publisher 在这里创建
    // gimbal_pose_pub_：
    //   gimbal_pose_pub_ = node_.create_publisher<decision_interfaces::msg::GimbalPose>(
    //       _gimbal_pose_topic, rclcpp::QoS(10));
    // 并把话题名补进下面的 RCLCPP_INFO，方便启动时确认接线。
    RCLCPP_INFO(
        node_.get_logger(), "SentryCmdEgress: blackboard-driven cmd=%s spin=%s energy_aim=%s",
        _sentry_cmd_topic.c_str(), _use_spin_topic.c_str(), _energy_aim_topic.c_str());
}


void SentryCmdEgress::publish(const BT::Blackboard & _bb)
{
    decision_interfaces::msg::SentryCmd cmd;

    bool revive = false;
    (void)_bb.get(BbKey::kReviveRequest, revive);
    cmd.resurrection_en = revive;
    cmd.buy_resurrection_en = false;

    std::uint16_t ammo_total = 0;
    (void)_bb.get(BbKey::kAmmoPurchaseTotal, ammo_total);
    cmd.buy_projectile_allowance = ammo_total;
    std::uint16_t buy_proj_times = 0;
    (void)_bb.get(BbKey::kBuyProjectileTimes, buy_proj_times);
    cmd.buy_projectile_times = buy_proj_times;

    std::uint16_t buy_hp = 0;
    (void)_bb.get(BbKey::kBuyHpTimes, buy_hp);
    cmd.buy_hp_times = buy_hp;

    std::uint8_t stance = static_cast<std::uint8_t>(StanceCmd_e::Move);
    (void)_bb.get(BbKey::kDesiredStance, stance);
    // SentryCmd posture_cmd only supports 1..3; clamp enhanced 4..6
    if (stance >= 4 && stance <= 6) {
        stance = static_cast<std::uint8_t>(stance - 3);
    }
    if (stance < 1 || stance > 3) {
        stance = 3;
    }
    cmd.posture_cmd = stance;

    cmd.energy_activation_confirm = false;

    bool super_cap = false;
    (void)_bb.get(BbKey::kUseSuperCap, super_cap);
    cmd.use_super_cap = super_cap;

    cmd.small_camera_control = false;
    sentry_pub_->publish(cmd);

    bool use_spin = false;
    (void)_bb.get(BbKey::kUseSpin, use_spin);
    std_msgs::msg::Bool spin_msg;
    spin_msg.data = use_spin;
    spin_pub_->publish(spin_msg);

    std::uint8_t energy_aim = 0;
    (void)_bb.get(BbKey::kEnergyAimMode, energy_aim);
    std_msgs::msg::UInt8 energy_aim_msg;
    energy_aim_msg.data = energy_aim;
    energy_aim_pub_->publish(energy_aim_msg);

    // TODO: 把云台姿态发给导航（/nav/gimbal_pose，类型 decision_interfaces/msg/GimbalPose）。
    // 接口就用 GimbalPose，enable 字段是启用/关闭云台控制的开关，
    // 不是控制模式枚举，也不用 rmoss_interfaces/GimbalCmd。
    // 注意该消息目前全仓库没有任何订阅方，导航侧需要自己实现接收端，否则发出去没人接。
    // 云台子树已经把决策结果写进 blackboard，这里只差最后一步发布：
    //   float gimbal_yaw = 0.f, gimbal_pitch = 0.f;
    //   bool gimbal_enabled = false;
    //   (void)_bb.get(BbKey::kGimbalYawCmd, gimbal_yaw);
    //   (void)_bb.get(BbKey::kGimbalPitchCmd, gimbal_pitch);
    //   (void)_bb.get(BbKey::kGimbalEnabled, gimbal_enabled);   // 开关，语义见 gimbal_tree.xml
    //   decision_interfaces::msg::GimbalPose gimbal_msg;
    //   gimbal_msg.enable = gimbal_enabled;
    //   gimbal_msg.yaw = gimbal_yaw;
    //   gimbal_msg.pitch = gimbal_pitch;
    //   gimbal_pose_pub_->publish(gimbal_msg);
    //   enable=false 时建议只发一次，避免导航侧反复收到「关闭」而反复重置云台；
    //   enable=true 时才每 tick 重发。
}

} // namespace rm_decision
