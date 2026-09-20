#include "rm_decision/infrastructure/game_info_referee_ingress.hpp"

#include "rm_decision/domain/blackboard_keys.hpp"

namespace rm_decision
{

GameInfoRefereeIngress::GameInfoRefereeIngress(rclcpp::Node & _node, const std::string & _topic)
: node_(_node)
{
  const auto qos = rclcpp::QoS(rclcpp::KeepLast(10)).reliable();
  sub_ = node_.create_subscription<decision_interfaces::msg::GameInfo>(
    _topic, qos, [this](const decision_interfaces::msg::GameInfo::SharedPtr msg) {
      onGameInfo_(msg);
    });
  RCLCPP_INFO(node_.get_logger(), "GameInfoRefereeIngress: subscribe %s", _topic.c_str());
}


void GameInfoRefereeIngress::onGameInfo_(const decision_interfaces::msg::GameInfo::SharedPtr _msg)
{
  std::lock_guard<std::mutex> lock(mutex_);
  latest_ = *_msg;
}


void GameInfoRefereeIngress::update(BT::Blackboard & _bb)
{
  std::optional<decision_interfaces::msg::GameInfo> copy;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    copy = latest_;
  }

  if (!copy) {
    _bb.set(BbKey::kHealth, 0.0f);
    _bb.set(BbKey::kCurrentHp, static_cast<std::uint16_t>(0));
    _bb.set(BbKey::kGameProgress, static_cast<std::uint8_t>(0));
    return;
  }

  const auto & g = *copy;
  const float health = static_cast<float>(g.current_hp);
  _bb.set(BbKey::kHealth, health);
  _bb.set(BbKey::kCurrentHp, g.current_hp);
  _bb.set(BbKey::kGameProgress, g.game_progress);
  _bb.set(BbKey::kGameTimeRemaining, static_cast<int>(g.game_remaining_time));
  _bb.set(BbKey::kRemainingEnergy, g.remaining_energy);
  _bb.set(BbKey::kAllyOutpostHp, g.ally_outpost_hp);
  _bb.set(BbKey::kAllyBaseHp, g.ally_base_hp);
  _bb.set(BbKey::kEnemyOutpostHp, g.enemy_outpost_hp);
  _bb.set(BbKey::kEnemyBaseHp, g.enemy_base_hp);
  _bb.set(BbKey::kHurtHpDeductionReason, g.hurt_hp_deduction_reason);
  _bb.set(BbKey::kPostureFeedback, g.posture);
  _bb.set(BbKey::kOutOfCombat, g.out_of_combat);
  _bb.set(BbKey::kIsDisengaged, g.out_of_combat);
  _bb.set(BbKey::kRedeemable17mm, g.redeemable_17mm);
  _bb.set(BbKey::kBulletsRemaining, static_cast<int>(g.bullet_remaining_17mm));
  _bb.set(BbKey::kEnergyActivatable, g.energy_activatable);
  _bb.set(BbKey::kCoinRemaining, static_cast<int>(g.coin_remaining));
  _bb.set(BbKey::kCurrentHeat, static_cast<int>(g.current_heat));
  _bb.set(BbKey::kHeatLimit, static_cast<int>(g.heat_limit));
  _bb.set(BbKey::kBigEnergyStatus, g.big_energy_status);
  _bb.set(BbKey::kControlMode, g.control_mode);
  _bb.set(BbKey::kManualGoalX, static_cast<double>(g.manual_goal_x));
  _bb.set(BbKey::kManualGoalY, static_cast<double>(g.manual_goal_y));
  _bb.set(BbKey::kCanFreeResurrect, g.can_free_resurrect);
}

}  // namespace rm_decision
