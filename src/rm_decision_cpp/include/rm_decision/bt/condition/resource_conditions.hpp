#pragma once

#include "rm_decision/domain/zone_map.hpp"

#include <behaviortree_cpp/condition_node.h>

#include <string>

namespace rm_decision
{

class CheckHealth : public BT::ConditionNode
{
public:
  CheckHealth(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

class CheckCoinRemaining : public BT::ConditionNode
{
public:
  CheckCoinRemaining(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

class CheckCanFreeResurrect : public BT::ConditionNode
{
public:
  CheckCanFreeResurrect(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts() { return {}; }
  BT::NodeStatus tick() override;
};

class CheckInZone : public BT::ConditionNode
{
public:
  CheckInZone(const std::string & _name, const BT::NodeConfig & _config, const ZoneMap * _zones);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;

private:
  const ZoneMap * zones_;
};

class CheckEngagedSafeResponse : public BT::ConditionNode
{
public:
  CheckEngagedSafeResponse(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;

private:
  double engaged_since_sec_{-1.0};
};

class CheckRemoteExchangeCooldown : public BT::ConditionNode
{
public:
  CheckRemoteExchangeCooldown(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

class CheckRemainingAmmoExchange : public BT::ConditionNode
{
public:
  CheckRemainingAmmoExchange(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

class CheckAttackFortHealthExchangeNeeded : public BT::ConditionNode
{
public:
  CheckAttackFortHealthExchangeNeeded(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;
};

class CheckNormalExchangeCooldown : public BT::ConditionNode
{
public:
  CheckNormalExchangeCooldown(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts();
  BT::NodeStatus tick() override;

private:
  double last_exchange_sec_{-1.0e9};
};

class CheckEnergyActive : public BT::ConditionNode
{
public:
  CheckEnergyActive(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts() { return {}; }
  BT::NodeStatus tick() override;
};

class CheckCanActivateEnergy : public BT::ConditionNode
{
public:
  CheckCanActivateEnergy(const std::string & _name, const BT::NodeConfig & _config);
  static BT::PortsList providedPorts() { return {}; }
  BT::NodeStatus tick() override;
};

}  // namespace rm_decision
