#include "rm_decision/infrastructure/sentry_cmd_egress.hpp"

#include "rm_decision/domain/blackboard_keys.hpp"

#include <cstdint>

namespace rm_decision
{

SentryCmdEgress::SentryCmdEgress(
  rclcpp::Node & _node, const std::string & _sentry_cmd_topic, const std::string & _use_spin_topic)
: node_(_node)
{
  sentry_pub_ = node_.create_publisher<decision_interfaces::msg::SentryCmd>(
    _sentry_cmd_topic, rclcpp::QoS(1).transient_local());
  spin_pub_ = node_.create_publisher<std_msgs::msg::Bool>(_use_spin_topic, rclcpp::QoS(10));
  RCLCPP_INFO(
    node_.get_logger(), "SentryCmdEgress: blackboard-driven cmd=%s spin=%s",
    _sentry_cmd_topic.c_str(), _use_spin_topic.c_str());
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
  // Also auto-enable in enemy fort if stance tree missed it
  bool in_fort = false;
  (void)_bb.get(BbKey::kInEnemyFortZone, in_fort);
  cmd.use_super_cap = super_cap || in_fort;

  cmd.small_camera_control = false;
  sentry_pub_->publish(cmd);

  bool use_spin = false;
  (void)_bb.get(BbKey::kUseSpin, use_spin);
  std_msgs::msg::Bool spin_msg;
  spin_msg.data = use_spin;
  spin_pub_->publish(spin_msg);
}

}  // namespace rm_decision
