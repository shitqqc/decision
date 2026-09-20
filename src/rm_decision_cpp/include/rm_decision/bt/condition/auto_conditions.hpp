#pragma once

#include "rm_decision/domain/zone_map.hpp"

#include <behaviortree_cpp/condition_node.h>

#include <chrono>
#include <cstdint>
#include <string>

namespace rm_decision
{

class IsInMatch : public BT::ConditionNode
{
public:
  IsInMatch(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts() { return {}; }
  BT::NodeStatus tick() override;
};

class IsDeadInMatch : public BT::ConditionNode
{
public:
  IsDeadInMatch(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts() { return {}; }
  BT::NodeStatus tick() override;
};

class CheckGameTimeWindow : public BT::ConditionNode
{
public:
  CheckGameTimeWindow(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

class CheckBigEnergyActive : public BT::ConditionNode
{
public:
  CheckBigEnergyActive(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

class CheckManualControl : public BT::ConditionNode
{
public:
  CheckManualControl(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts() { return {}; }
  BT::NodeStatus tick() override;
};

/// Alias of bit CheckManualOverride.
using CheckManualOverride = CheckManualControl;

class CheckManualFortAttack : public BT::ConditionNode
{
public:
  CheckManualFortAttack(
    const std::string & _name, const BT::NodeConfig & _config, const ZoneMap * _zones);
  static BT::PortsList providedPorts() { return {}; }
  BT::NodeStatus tick() override;

private:
  const ZoneMap * zones_;
};

class CheckTacticalMode : public BT::ConditionNode
{
public:
  CheckTacticalMode(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

/// Alias of bit CheckTacticalModeCondition.
using CheckTacticalModeCondition = CheckTacticalMode;

class CheckAmmoLow : public BT::ConditionNode
{
public:
  CheckAmmoLow(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

class CheckOutpostRemained : public BT::ConditionNode
{
public:
  CheckOutpostRemained(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

class CheckOwnOutpostAlive : public BT::ConditionNode
{
public:
  CheckOwnOutpostAlive(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

class CheckRetreatCondition : public BT::ConditionNode
{
public:
  CheckRetreatCondition(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

class UpdateOutpostAttackState : public BT::ConditionNode
{
public:
  UpdateOutpostAttackState(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;

private:
  enum class Phase { Waiting, AutoAttack, Retreat, Done };
  Phase phase_{Phase::Waiting};
  int retreat_count_{0};
  int previous_game_progress_{0};
  bool pregame_reset_done_{false};
  std::chrono::steady_clock::time_point retreat_start_{};
};

class CheckOutpostAttackState : public BT::ConditionNode
{
public:
  CheckOutpostAttackState(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

class SetEnemyOutpostDestroyed : public BT::ConditionNode
{
public:
  SetEnemyOutpostDestroyed(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

class CheckHeroGuardActive : public BT::ConditionNode
{
public:
  CheckHeroGuardActive(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts() { return {}; }
  BT::NodeStatus tick() override;
};

class SetHeroGuardActive : public BT::ConditionNode
{
public:
  SetHeroGuardActive(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

class UpdateHighlandFallbackState : public BT::ConditionNode
{
public:
  UpdateHighlandFallbackState(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

class CheckHighlandFallbackActive : public BT::ConditionNode
{
public:
  CheckHighlandFallbackActive(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts() { return {}; }
  BT::NodeStatus tick() override;
};

class CheckOutpostSafeResponse : public BT::ConditionNode
{
public:
  CheckOutpostSafeResponse(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

class CheckTargetLocked : public BT::ConditionNode
{
public:
  CheckTargetLocked(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts() { return {}; }
  BT::NodeStatus tick() override;
};

class CheckTargetArmorId : public BT::ConditionNode
{
public:
  CheckTargetArmorId(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

class CheckInStairsZone : public BT::ConditionNode
{
public:
  CheckInStairsZone(
    const std::string & _name, const BT::NodeConfig & _config, const ZoneMap * _zones);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;

private:
  const ZoneMap * zones_;
};

class CheckNoAllyBelowStairs : public BT::ConditionNode
{
public:
  CheckNoAllyBelowStairs(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts() { return {}; }
  BT::NodeStatus tick() override;
};

class CheckOwnFortIdle : public BT::ConditionNode
{
public:
  CheckOwnFortIdle(
    const std::string & _name, const BT::NodeConfig & _config, const ZoneMap * _zones);
  static BT::PortsList providedPorts() { return {}; }
  BT::NodeStatus tick() override;

private:
  const ZoneMap * zones_;
};

}  // namespace rm_decision
