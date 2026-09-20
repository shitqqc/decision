#include "rm_decision/bt/condition/resource_conditions.hpp"

#include "rm_decision/domain/blackboard_keys.hpp"

#include <chrono>
#include <cstdint>

namespace rm_decision
{
namespace {
double nowSec()
{
  const auto now = std::chrono::steady_clock::now().time_since_epoch();
  return std::chrono::duration_cast<std::chrono::duration<double>>(now).count();
}
}  // namespace

CheckHealth::CheckHealth(const std::string & n, const BT::NodeConfig & c) : ConditionNode(n, c) {}
BT::PortsList CheckHealth::providedPorts()
{
  return {BT::InputPort<float>("threshold", 0.0f, ""), BT::InputPort<std::string>("mode", "less", "")};
}
BT::NodeStatus CheckHealth::tick()
{
  float threshold = 0.0f; std::string mode = "less";
  (void)getInput("threshold", threshold); (void)getInput("mode", mode);
  float health = 0.0f; (void)config().blackboard->get(BbKey::kHealth, health);
  bool ok = (mode == "greater") ? (health > threshold) : (mode == "less_equal") ? (health <= threshold) : (health < threshold);
  return ok ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

CheckCoinRemaining::CheckCoinRemaining(const std::string & n, const BT::NodeConfig & c) : ConditionNode(n, c) {}
BT::PortsList CheckCoinRemaining::providedPorts()
{
  return {BT::InputPort<int>("threshold", 100, ""), BT::InputPort<std::string>("mode", "greater", "")};
}
BT::NodeStatus CheckCoinRemaining::tick()
{
  int thr = 100; std::string mode = "greater";
  (void)getInput("threshold", thr); (void)getInput("mode", mode);
  int coin = 0; (void)config().blackboard->get(BbKey::kCoinRemaining, coin);
  return ((mode == "less") ? (coin < thr) : (coin > thr)) ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

CheckCanFreeResurrect::CheckCanFreeResurrect(const std::string & n, const BT::NodeConfig & c) : ConditionNode(n, c) {}
BT::NodeStatus CheckCanFreeResurrect::tick()
{
  bool can = true; (void)config().blackboard->get(BbKey::kCanFreeResurrect, can);
  float health = 1.0f; std::uint8_t progress = 0;
  (void)config().blackboard->get(BbKey::kHealth, health);
  (void)config().blackboard->get(BbKey::kGameProgress, progress);
  if (progress == 4u && health <= 0.0f) return BT::NodeStatus::SUCCESS;
  return can ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

CheckInZone::CheckInZone(const std::string & n, const BT::NodeConfig & c, const ZoneMap * z)
: ConditionNode(n, c), zones_(z) {}
BT::PortsList CheckInZone::providedPorts() { return {BT::InputPort<std::string>("zone_name", "", "")}; }
BT::NodeStatus CheckInZone::tick()
{
  if (!zones_) return BT::NodeStatus::FAILURE;
  std::string name; if (!getInput("zone_name", name)) return BT::NodeStatus::FAILURE;
  if (name == "enemy_fort" || name == "enemy_fort_zone") {
    bool v=false; (void)config().blackboard->get(BbKey::kInEnemyFortZone, v);
    return v ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
  }
  if (name == "own_supply" || name == "own_supply_zone") {
    bool v=false; (void)config().blackboard->get(BbKey::kInOwnSupplyZone, v);
    return v ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
  }
  if (name == "own_outpost" || name == "own_outpost_zone") {
    bool v=false; (void)config().blackboard->get(BbKey::kInOwnOutpostZone, v);
    return v ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
  }
  bool pose_valid=false; double x=0,y=0;
  (void)config().blackboard->get(BbKey::kPoseValid, pose_valid);
  (void)config().blackboard->get(BbKey::kCurrentPoseX, x);
  (void)config().blackboard->get(BbKey::kCurrentPoseY, y);
  if (!pose_valid) return BT::NodeStatus::FAILURE;
  return zones_->inZone(name, x, y) ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

CheckEngagedSafeResponse::CheckEngagedSafeResponse(const std::string & n, const BT::NodeConfig & c) : ConditionNode(n, c) {}
BT::PortsList CheckEngagedSafeResponse::providedPorts()
{
  return {BT::InputPort<double>("safe_duration_sec", 3.0, ""), BT::InputPort<bool>("expected_engaged", true, "")};
}
BT::NodeStatus CheckEngagedSafeResponse::tick()
{
  double dur=3.0; bool expected=true;
  (void)getInput("safe_duration_sec", dur); (void)getInput("expected_engaged", expected);
  bool disengaged=false; (void)config().blackboard->get(BbKey::kIsDisengaged, disengaged);
  const bool engaged = !disengaged;
  if (engaged != expected) { engaged_since_sec_ = -1.0; return BT::NodeStatus::FAILURE; }
  const double t = nowSec();
  if (engaged_since_sec_ < 0) engaged_since_sec_ = t;
  return (t - engaged_since_sec_ >= dur) ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

CheckRemoteExchangeCooldown::CheckRemoteExchangeCooldown(const std::string & n, const BT::NodeConfig & c) : ConditionNode(n, c) {}
BT::PortsList CheckRemoteExchangeCooldown::providedPorts()
{
  return {
    BT::InputPort<int>("max_count", 3, ""),
    BT::InputPort<std::string>("count_key", "remote_ammo_exchange_count", ""),
    BT::InputPort<std::string>("exchange_count_key", "remote_ammo_exchange_count", ""),
    BT::InputPort<double>("cooldown_seconds", 0.0, ""),
  };
}
BT::NodeStatus CheckRemoteExchangeCooldown::tick()
{
  int max_count = 3;
  std::string key = BbKey::kRemoteAmmoExchangeCount;
  (void)getInput("max_count", max_count);
  if (!getInput("exchange_count_key", key)) (void)getInput("count_key", key);
  (void)getInput<double>("cooldown_seconds");
  int count = 0;
  (void)config().blackboard->get(key, count);
  return count < max_count ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

CheckRemainingAmmoExchange::CheckRemainingAmmoExchange(const std::string & n, const BT::NodeConfig & c) : ConditionNode(n, c) {}
BT::PortsList CheckRemainingAmmoExchange::providedPorts()
{
  return {BT::InputPort<int>("min_remaining", 1, "")};
}
BT::NodeStatus CheckRemainingAmmoExchange::tick()
{
  int min_r=1; (void)getInput("min_remaining", min_r);
  int rem=0; (void)config().blackboard->get(BbKey::kRemainingAmmoExchange, rem);
  if (rem == 0) { (void)config().blackboard->get(BbKey::kRedeemable17mm, rem); }
  return rem >= min_r ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

CheckAttackFortHealthExchangeNeeded::CheckAttackFortHealthExchangeNeeded(const std::string & n, const BT::NodeConfig & c) : ConditionNode(n, c) {}
BT::PortsList CheckAttackFortHealthExchangeNeeded::providedPorts()
{
  return {BT::InputPort<float>("hp_threshold", 200.0f, "")};
}
BT::NodeStatus CheckAttackFortHealthExchangeNeeded::tick()
{
  float thr=200.f; (void)getInput("hp_threshold", thr);
  std::uint8_t tm=0; (void)config().blackboard->get(BbKey::kTacticalMode, tm);
  if (tm != static_cast<std::uint8_t>(TacticalMode_e::Attack)) return BT::NodeStatus::FAILURE;
  float health=0; (void)config().blackboard->get(BbKey::kHealth, health);
  return health < thr ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

CheckNormalExchangeCooldown::CheckNormalExchangeCooldown(const std::string & n, const BT::NodeConfig & c) : ConditionNode(n, c) {}
BT::PortsList CheckNormalExchangeCooldown::providedPorts()
{
  return {
    BT::InputPort<double>("cooldown_sec", 5.0, ""),
    BT::InputPort<double>("cooldown_seconds", 5.0, ""),
  };
}
BT::NodeStatus CheckNormalExchangeCooldown::tick()
{
  double cd=5.0;
  if(!getInput("cooldown_seconds", cd)) (void)getInput("cooldown_sec", cd);
  const double t=nowSec();
  if (last_exchange_sec_ > 0 && (t - last_exchange_sec_) < cd) return BT::NodeStatus::FAILURE;
  last_exchange_sec_ = t;
  return BT::NodeStatus::SUCCESS;
}

CheckEnergyActive::CheckEnergyActive(const std::string & n, const BT::NodeConfig & c) : ConditionNode(n, c) {}
BT::NodeStatus CheckEnergyActive::tick()
{
  std::uint8_t st=0; (void)config().blackboard->get(BbKey::kBigEnergyStatus, st);
  return st != 0 ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

CheckCanActivateEnergy::CheckCanActivateEnergy(const std::string & n, const BT::NodeConfig & c) : ConditionNode(n, c) {}
BT::NodeStatus CheckCanActivateEnergy::tick()
{
  bool v=false; (void)config().blackboard->get(BbKey::kEnergyActivatable, v);
  return v ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

}  // namespace rm_decision
