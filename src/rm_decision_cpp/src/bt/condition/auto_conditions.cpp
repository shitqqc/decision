#include "rm_decision/bt/condition/auto_conditions.hpp"

#include "rm_decision/domain/blackboard_keys.hpp"
#include <chrono>

#include <cstdint>

namespace rm_decision
{

IsInMatch::IsInMatch(const std::string & n, const BT::NodeConfig & c) : ConditionNode(n, c) {}
BT::NodeStatus IsInMatch::tick()
{
  std::uint8_t p=0; (void)config().blackboard->get(BbKey::kGameProgress, p);
  return p==4u ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

IsDeadInMatch::IsDeadInMatch(const std::string & n, const BT::NodeConfig & c) : ConditionNode(n, c) {}
BT::NodeStatus IsDeadInMatch::tick()
{
  float h=1; std::uint8_t p=0;
  (void)config().blackboard->get(BbKey::kHealth, h);
  (void)config().blackboard->get(BbKey::kGameProgress, p);
  return (p==4u && h<=0.f) ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

CheckGameTimeWindow::CheckGameTimeWindow(const std::string & n, const BT::NodeConfig & c) : ConditionNode(n, c) {}
BT::PortsList CheckGameTimeWindow::providedPorts()
{
  return {
    BT::InputPort<int>("min_remaining", 0, ""),
    BT::InputPort<int>("max_remaining", 420, ""),
    BT::InputPort<bool>("require_game_started", true, ""),
  };
}
BT::NodeStatus CheckGameTimeWindow::tick()
{
  int min_r = 0, max_r = 420;
  bool require = true;
  (void)getInput("min_remaining", min_r);
  (void)getInput("max_remaining", max_r);
  (void)getInput("require_game_started", require);
  int t = 0;
  std::uint8_t p = 0;
  (void)config().blackboard->get(BbKey::kGameTimeRemaining, t);
  (void)config().blackboard->get(BbKey::kGameProgress, p);
  if (require && p != 4u) return BT::NodeStatus::FAILURE;
  return (t >= min_r && t <= max_r) ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

CheckBigEnergyActive::CheckBigEnergyActive(const std::string & n, const BT::NodeConfig & c) : ConditionNode(n, c) {}
BT::PortsList CheckBigEnergyActive::providedPorts()
{
  return {BT::InputPort<int>("active_status",1,"")};
}
BT::NodeStatus CheckBigEnergyActive::tick()
{
  int expect=1; (void)getInput("active_status",expect);
  std::uint8_t st=0; (void)config().blackboard->get(BbKey::kBigEnergyStatus,st);
  if (st==0) {
    bool a=false; (void)config().blackboard->get(BbKey::kEnergyActivatable,a);
    if (a && expect==1) return BT::NodeStatus::SUCCESS;
  }
  return static_cast<int>(st)==expect ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

CheckManualControl::CheckManualControl(const std::string & n, const BT::NodeConfig & c) : ConditionNode(n, c) {}
BT::NodeStatus CheckManualControl::tick()
{
  std::uint8_t m=0; (void)config().blackboard->get(BbKey::kControlMode,m);
  return m==static_cast<std::uint8_t>(ControlMode_e::Manual) ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

CheckManualFortAttack::CheckManualFortAttack(const std::string & n, const BT::NodeConfig & c, const ZoneMap * z)
: ConditionNode(n,c), zones_(z) {}
BT::NodeStatus CheckManualFortAttack::tick()
{
  std::uint8_t m=0; (void)config().blackboard->get(BbKey::kControlMode,m);
  if (m!=static_cast<std::uint8_t>(ControlMode_e::Manual) || !zones_) return BT::NodeStatus::FAILURE;
  double x=0,y=0; (void)config().blackboard->get(BbKey::kManualGoalX,x); (void)config().blackboard->get(BbKey::kManualGoalY,y);
  return zones_->inZone("enemy_fort",x,y) ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

CheckTacticalMode::CheckTacticalMode(const std::string & n, const BT::NodeConfig & c) : ConditionNode(n, c) {}
BT::PortsList CheckTacticalMode::providedPorts()
{
  return {BT::InputPort<std::string>("mode","normal","")};
}
BT::NodeStatus CheckTacticalMode::tick()
{
  std::string want="normal"; (void)getInput("mode",want);
  std::uint8_t mode=0; (void)config().blackboard->get(BbKey::kTacticalMode,mode);
  TacticalMode_e expect=TacticalMode_e::Normal;
  if (want=="attack") expect=TacticalMode_e::Attack;
  else if (want=="defend") expect=TacticalMode_e::Defend;
  return mode==static_cast<std::uint8_t>(expect) ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

CheckAmmoLow::CheckAmmoLow(const std::string & n, const BT::NodeConfig & c) : ConditionNode(n, c) {}
BT::PortsList CheckAmmoLow::providedPorts() { return {BT::InputPort<int>("ammo_threshold",150,"")}; }
BT::NodeStatus CheckAmmoLow::tick()
{
  int thr=150; (void)getInput("ammo_threshold",thr);
  int ammo=0; (void)config().blackboard->get(BbKey::kBulletsRemaining,ammo);
  return ammo<thr ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

CheckOutpostRemained::CheckOutpostRemained(const std::string & n, const BT::NodeConfig & c) : ConditionNode(n, c) {}
BT::PortsList CheckOutpostRemained::providedPorts() { return {BT::InputPort<float>("min_hp",1.f,"")}; }
BT::NodeStatus CheckOutpostRemained::tick()
{
  float min_hp=1.f; (void)getInput("min_hp",min_hp);
  float hp=0; (void)config().blackboard->get(BbKey::kEnemyOutpostHp,hp);
  bool destroyed=false; (void)config().blackboard->get(BbKey::kEnemyOutpostDestroyed,destroyed);
  if (destroyed) return BT::NodeStatus::FAILURE;
  return hp>=min_hp ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

CheckOwnOutpostAlive::CheckOwnOutpostAlive(const std::string & n, const BT::NodeConfig & c) : ConditionNode(n, c) {}
BT::PortsList CheckOwnOutpostAlive::providedPorts() { return {BT::InputPort<float>("min_hp",1.f,"")}; }
BT::NodeStatus CheckOwnOutpostAlive::tick()
{
  float min_hp=1.f; (void)getInput("min_hp",min_hp);
  float hp=0; (void)config().blackboard->get(BbKey::kAllyOutpostHp,hp);
  return hp>=min_hp ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

CheckRetreatCondition::CheckRetreatCondition(const std::string & n, const BT::NodeConfig & c) : ConditionNode(n, c) {}
BT::PortsList CheckRetreatCondition::providedPorts()
{
  return {
    BT::InputPort<float>("health_threshold", 50.0f, ""),
    BT::InputPort<float>("recovery_threshold", 50.0f, ""),
  };
}
BT::NodeStatus CheckRetreatCondition::tick()
{
  float health_threshold = 50.0f;
  float recovery_threshold = 50.0f;
  (void)getInput("health_threshold", health_threshold);
  (void)getInput("recovery_threshold", recovery_threshold);
  float health = 0.0f;
  (void)config().blackboard->get(BbKey::kHealth, health);
  std::uint8_t mode = static_cast<std::uint8_t>(NavMode_e::Patrol);
  (void)config().blackboard->get(BbKey::kNavMode, mode);
  if (mode == static_cast<std::uint8_t>(NavMode_e::Retreat)) {
    if (health >= recovery_threshold) {
      config().blackboard->set(BbKey::kNavMode, static_cast<std::uint8_t>(NavMode_e::Patrol));
      return BT::NodeStatus::FAILURE;
    }
    return BT::NodeStatus::SUCCESS;
  }
  if (health < health_threshold) {
    config().blackboard->set(BbKey::kNavMode, static_cast<std::uint8_t>(NavMode_e::Retreat));
    return BT::NodeStatus::SUCCESS;
  }
  return BT::NodeStatus::FAILURE;
}

UpdateOutpostAttackState::UpdateOutpostAttackState(const std::string & n, const BT::NodeConfig & c) : ConditionNode(n, c) {}
BT::PortsList UpdateOutpostAttackState::providedPorts()
{
  return {
    BT::InputPort<float>("retreat_health_threshold", 50.0f, ""),
    BT::InputPort<float>("recovery_health_threshold", 90.0f, ""),
    BT::InputPort<double>("enhanced_defend_seconds", 5.0, ""),
    BT::InputPort<int>("max_retreat_count", 3, ""),
  };
}
BT::NodeStatus UpdateOutpostAttackState::tick()
{
  float retreat_hp = 50.0f, recovery_hp = 90.0f;
  double defend_s = 5.0;
  int max_retreat = 3;
  (void)getInput("retreat_health_threshold", retreat_hp);
  (void)getInput("recovery_health_threshold", recovery_hp);
  (void)getInput("enhanced_defend_seconds", defend_s);
  (void)getInput("max_retreat_count", max_retreat);

  std::uint8_t progress = 0;
  int t_rem = 0;
  float enemy_op = 0.0f;
  float health = 0.0f;
  (void)config().blackboard->get(BbKey::kGameProgress, progress);
  (void)config().blackboard->get(BbKey::kGameTimeRemaining, t_rem);
  (void)config().blackboard->get(BbKey::kEnemyOutpostHp, enemy_op);
  (void)config().blackboard->get(BbKey::kHealth, health);
  const auto now = std::chrono::steady_clock::now();

  const bool pregame_reset = progress != 4u && t_rem >= 410;
  const bool new_match = progress == 4u && previous_game_progress_ != 4 && t_rem >= 410;
  previous_game_progress_ = static_cast<int>(progress);
  if ((pregame_reset && !pregame_reset_done_) || new_match) {
    phase_ = Phase::Waiting;
    retreat_count_ = 0;
    retreat_start_ = {};
    config().blackboard->set(BbKey::kEnemyOutpostDestroyed, false);
    config().blackboard->set(BbKey::kNavMode, static_cast<std::uint8_t>(NavMode_e::Patrol));
    pregame_reset_done_ = true;
  } else if (progress == 4u) {
    pregame_reset_done_ = false;
  }

  bool destroyed = false;
  (void)config().blackboard->get(BbKey::kEnemyOutpostDestroyed, destroyed);
  if (progress == 4u) {
    if (phase_ == Phase::Waiting) {
      if (enemy_op > 0.0f && !destroyed) phase_ = Phase::AutoAttack;
      else if (enemy_op <= 0.0f) {
        config().blackboard->set(BbKey::kEnemyOutpostDestroyed, true);
        destroyed = true;
        phase_ = Phase::Done;
      }
    } else if (phase_ == Phase::AutoAttack) {
      if (destroyed) phase_ = Phase::Done;
      else if (enemy_op <= 0.0f) {
        config().blackboard->set(BbKey::kEnemyOutpostDestroyed, true);
        config().blackboard->set(BbKey::kNavMode, static_cast<std::uint8_t>(NavMode_e::Patrol));
        phase_ = Phase::Done;
      } else if (health <= retreat_hp) {
        ++retreat_count_;
        retreat_start_ = now;
        phase_ = Phase::Retreat;
      }
    } else if (phase_ == Phase::Retreat) {
      if (enemy_op <= 0.0f && !destroyed) {
        config().blackboard->set(BbKey::kEnemyOutpostDestroyed, true);
        destroyed = true;
      }
      const double elapsed = std::chrono::duration<double>(now - retreat_start_).count();
      if (elapsed >= defend_s && health >= recovery_hp) {
        const bool finished = destroyed || enemy_op <= 0.0f || retreat_count_ >= max_retreat;
        config().blackboard->set(BbKey::kEnemyOutpostDestroyed, finished);
        config().blackboard->set(BbKey::kNavMode, static_cast<std::uint8_t>(NavMode_e::Patrol));
        phase_ = finished ? Phase::Done : Phase::AutoAttack;
      }
    }
  }

  const bool auto_attack = phase_ == Phase::AutoAttack;
  const bool retreat = phase_ == Phase::Retreat;
  const bool enhanced = retreat;  // simplified: enhanced while retreating
  config().blackboard->set(BbKey::kOutpostAutoAttackActive, auto_attack);
  config().blackboard->set(BbKey::kOutpostRetreatActive, retreat);
  config().blackboard->set(BbKey::kOutpostEnhancedDefendActive, enhanced);
  // manual_attack left for external writers
  return BT::NodeStatus::SUCCESS;
}

CheckOutpostAttackState::CheckOutpostAttackState(const std::string & n, const BT::NodeConfig & c) : ConditionNode(n, c) {}
BT::PortsList CheckOutpostAttackState::providedPorts()
{
  return {BT::InputPort<std::string>("state", "", "auto_attack|manual_attack|retreat|enhanced_defend")};
}
BT::NodeStatus CheckOutpostAttackState::tick()
{
  std::string state;
  if (!getInput("state", state)) return BT::NodeStatus::FAILURE;
  bool active = false;
  if (state == "auto_attack") (void)config().blackboard->get(BbKey::kOutpostAutoAttackActive, active);
  else if (state == "manual_attack") (void)config().blackboard->get(BbKey::kOutpostManualAttackActive, active);
  else if (state == "retreat") (void)config().blackboard->get(BbKey::kOutpostRetreatActive, active);
  else if (state == "enhanced_defend") (void)config().blackboard->get(BbKey::kOutpostEnhancedDefendActive, active);
  else return BT::NodeStatus::FAILURE;
  return active ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

SetEnemyOutpostDestroyed::SetEnemyOutpostDestroyed(const std::string & n, const BT::NodeConfig & c) : ConditionNode(n, c) {}
BT::PortsList SetEnemyOutpostDestroyed::providedPorts() { return {BT::InputPort<bool>("value",true,"")}; }
BT::NodeStatus SetEnemyOutpostDestroyed::tick()
{
  bool v=true; (void)getInput("value",v);
  config().blackboard->set(BbKey::kEnemyOutpostDestroyed, v);
  return BT::NodeStatus::SUCCESS;
}

CheckHeroGuardActive::CheckHeroGuardActive(const std::string & n, const BT::NodeConfig & c) : ConditionNode(n, c) {}
BT::NodeStatus CheckHeroGuardActive::tick()
{
  bool v=false; (void)config().blackboard->get(BbKey::kHeroGuardActive,v);
  return v ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

SetHeroGuardActive::SetHeroGuardActive(const std::string & n, const BT::NodeConfig & c) : ConditionNode(n, c) {}
BT::PortsList SetHeroGuardActive::providedPorts()
{
  return {BT::InputPort<bool>("active", true, ""), BT::InputPort<bool>("exit_nav_mode", false, "")};
}
BT::NodeStatus SetHeroGuardActive::tick()
{
  bool v = true;
  bool exit_nav = false;
  (void)getInput("active", v);
  (void)getInput("exit_nav_mode", exit_nav);
  config().blackboard->set(BbKey::kHeroGuardActive, v);
  if (exit_nav) {
    config().blackboard->set(BbKey::kNavMode, static_cast<std::uint8_t>(NavMode_e::Patrol));
  }
  return BT::NodeStatus::SUCCESS;
}

UpdateHighlandFallbackState::UpdateHighlandFallbackState(const std::string & n, const BT::NodeConfig & c) : ConditionNode(n, c) {}
BT::PortsList UpdateHighlandFallbackState::providedPorts()
{
  return {BT::InputPort<int>("cutoff_remaining", 0, "")};
}
BT::NodeStatus UpdateHighlandFallbackState::tick()
{
  int cutoff = 0;
  (void)getInput("cutoff_remaining", cutoff);
  float ally_base = 0;
  int t_rem = 0;
  (void)config().blackboard->get(BbKey::kAllyBaseHp, ally_base);
  (void)config().blackboard->get(BbKey::kGameTimeRemaining, t_rem);
  bool active = ally_base > 0.f && ally_base < 500.f;
  if (cutoff > 0 && t_rem < cutoff) active = false;
  config().blackboard->set(BbKey::kHighlandFallbackActive, active);
  return BT::NodeStatus::SUCCESS;
}

CheckHighlandFallbackActive::CheckHighlandFallbackActive(const std::string & n, const BT::NodeConfig & c) : ConditionNode(n, c) {}
BT::NodeStatus CheckHighlandFallbackActive::tick()
{
  bool v=false; (void)config().blackboard->get(BbKey::kHighlandFallbackActive,v);
  return v ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

CheckOutpostSafeResponse::CheckOutpostSafeResponse(const std::string & n, const BT::NodeConfig & c) : ConditionNode(n, c) {}
BT::PortsList CheckOutpostSafeResponse::providedPorts() { return {BT::InputPort<float>("min_enemy_outpost_hp",100.f,"")}; }
BT::NodeStatus CheckOutpostSafeResponse::tick()
{
  float min_hp=100.f; (void)getInput("min_enemy_outpost_hp",min_hp);
  float hp=0; (void)config().blackboard->get(BbKey::kEnemyOutpostHp,hp);
  return hp>=min_hp ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

CheckTargetLocked::CheckTargetLocked(const std::string & n, const BT::NodeConfig & c) : ConditionNode(n, c) {}
BT::NodeStatus CheckTargetLocked::tick()
{
  bool v=false; (void)config().blackboard->get(BbKey::kTargetValid,v);
  if (!v) config().blackboard->set(BbKey::kNotAimEnemy, true);
  return v ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

CheckTargetArmorId::CheckTargetArmorId(const std::string & n, const BT::NodeConfig & c) : ConditionNode(n, c) {}
BT::PortsList CheckTargetArmorId::providedPorts()
{
  return {BT::InputPort<int>("armor_id", 0, ""), BT::InputPort<int>("target_id", 0, "")};
}
BT::NodeStatus CheckTargetArmorId::tick()
{
  int want = 0;
  if (!getInput("target_id", want)) (void)getInput("armor_id", want);
  int id = -1;
  (void)config().blackboard->get(BbKey::kTargetArmorId, id);
  return id == want ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

CheckInStairsZone::CheckInStairsZone(const std::string & n, const BT::NodeConfig & c, const ZoneMap * z)
: ConditionNode(n,c), zones_(z) {}
BT::PortsList CheckInStairsZone::providedPorts() { return {BT::InputPort<std::string>("zone_name","stairs","")}; }
BT::NodeStatus CheckInStairsZone::tick()
{
  if (!zones_) return BT::NodeStatus::FAILURE;
  std::string name="stairs"; (void)getInput("zone_name",name);
  bool pose_valid=false; double x=0,y=0;
  (void)config().blackboard->get(BbKey::kPoseValid,pose_valid);
  (void)config().blackboard->get(BbKey::kCurrentPoseX,x);
  (void)config().blackboard->get(BbKey::kCurrentPoseY,y);
  if (!pose_valid) return BT::NodeStatus::FAILURE;
  return zones_->inZone(name,x,y) ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

CheckNoAllyBelowStairs::CheckNoAllyBelowStairs(const std::string & n, const BT::NodeConfig & c) : ConditionNode(n, c) {}
BT::NodeStatus CheckNoAllyBelowStairs::tick()
{
  // TODO(bit-port): needs allies_info ingress
  return BT::NodeStatus::SUCCESS;
}

CheckOwnFortIdle::CheckOwnFortIdle(const std::string & n, const BT::NodeConfig & c, const ZoneMap * z)
: ConditionNode(n,c), zones_(z) {}
BT::NodeStatus CheckOwnFortIdle::tick()
{
  if (!zones_) return BT::NodeStatus::FAILURE;
  bool pose_valid=false; double x=0,y=0;
  (void)config().blackboard->get(BbKey::kPoseValid,pose_valid);
  (void)config().blackboard->get(BbKey::kCurrentPoseX,x);
  (void)config().blackboard->get(BbKey::kCurrentPoseY,y);
  if (!pose_valid) return BT::NodeStatus::FAILURE;
  // Approximate: near own fort nav point index 3
  Point2d_s pt; if (!zones_->getNavPoint(3,&pt)) return BT::NodeStatus::FAILURE;
  const double dx=x-pt.x, dy=y-pt.y;
  return (dx*dx+dy*dy)<4.0 ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

}  // namespace rm_decision
