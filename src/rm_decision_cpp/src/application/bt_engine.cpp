#include "rm_decision/application/bt_engine.hpp"

#include "rm_decision/bt/action/auto_actions.hpp"
#include "rm_decision/bt/action/change_stance_action.hpp"
#include "rm_decision/bt/action/gimbal_action.hpp"
#include "rm_decision/bt/action/nav_action.hpp"
#include "rm_decision/bt/action/recovery_actions.hpp"
#include "rm_decision/bt/action/resource_actions.hpp"
#include "rm_decision/bt/action/tactical_action.hpp"
#include "rm_decision/bt/condition/auto_conditions.hpp"
#include "rm_decision/bt/condition/change_stance_condition.hpp"
#include "rm_decision/bt/condition/gimbal_condition.hpp"
#include "rm_decision/bt/condition/resource_conditions.hpp"
#include "rm_decision/bt/condition/tactical_condition.hpp"

#include <stdexcept>

namespace rm_decision
{

void BtEngine::registerNodes_(const ZoneMap * zones, rclcpp::Node::SharedPtr node)
{
  auto & factory = factory_;
  // ---- auto conditions ----
  factory.registerNodeType<IsInMatch>("IsInMatch");
  factory.registerNodeType<IsDeadInMatch>("IsDeadInMatch");
  factory.registerNodeType<CheckGameTimeWindow>("CheckGameTimeWindow");
  factory.registerNodeType<CheckBigEnergyActive>("CheckBigEnergyActive");
  factory.registerNodeType<CheckManualControl>("CheckManualControl");
  factory.registerNodeType<CheckManualControl>("CheckManualOverride");
  factory.registerNodeType<CheckManualFortAttack>("CheckManualFortAttack", zones);
  factory.registerNodeType<CheckTacticalMode>("CheckTacticalMode");
  factory.registerNodeType<CheckTacticalMode>("CheckTacticalModeCondition");
  factory.registerNodeType<CheckAmmoLow>("CheckAmmoLow");
  factory.registerNodeType<CheckOutpostRemained>("CheckOutpostRemained");
  factory.registerNodeType<CheckOwnOutpostAlive>("CheckOwnOutpostAlive");
  factory.registerNodeType<CheckRetreatCondition>("CheckRetreatCondition");
  factory.registerNodeType<UpdateOutpostAttackState>("UpdateOutpostAttackState");
  factory.registerNodeType<CheckOutpostAttackState>("CheckOutpostAttackState");
  factory.registerNodeType<SetEnemyOutpostDestroyed>("SetEnemyOutpostDestroyed");
  factory.registerNodeType<CheckHeroGuardActive>("CheckHeroGuardActive");
  factory.registerNodeType<SetHeroGuardActive>("SetHeroGuardActive");
  factory.registerNodeType<UpdateHighlandFallbackState>("UpdateHighlandFallbackState");
  factory.registerNodeType<CheckHighlandFallbackActive>("CheckHighlandFallbackActive");
  factory.registerNodeType<CheckOutpostSafeResponse>("CheckOutpostSafeResponse");
  factory.registerNodeType<CheckTargetLocked>("CheckTargetLocked");
  factory.registerNodeType<CheckTargetArmorId>("CheckTargetArmorId");
  factory.registerNodeType<CheckInStairsZone>("CheckInStairsZone", zones);
  factory.registerNodeType<CheckNoAllyBelowStairs>("CheckNoAllyBelowStairs");
  factory.registerNodeType<CheckOwnFortIdle>("CheckOwnFortIdle", zones);

  // ---- tactical / resource / stance / gimbal conditions ----
  factory.registerNodeType<CheckDefendCondition>("CheckDefendCondition");
  factory.registerNodeType<CheckAttackCondition>("CheckAttackCondition", zones);
  factory.registerNodeType<CheckHealth>("CheckHealth");
  factory.registerNodeType<CheckCoinRemaining>("CheckCoinRemaining");
  factory.registerNodeType<CheckCanFreeResurrect>("CheckCanFreeResurrect");
  factory.registerNodeType<CheckInZone>("CheckInZone", zones);
  factory.registerNodeType<CheckEngagedSafeResponse>("CheckEngagedSafeResponse");
  factory.registerNodeType<CheckRemoteExchangeCooldown>("CheckRemoteExchangeCooldown");
  factory.registerNodeType<CheckRemainingAmmoExchange>("CheckRemainingAmmoExchange");
  factory.registerNodeType<CheckAttackFortHealthExchangeNeeded>("CheckAttackFortHealthExchangeNeeded");
  factory.registerNodeType<CheckNormalExchangeCooldown>("CheckNormalExchangeCooldown");
  factory.registerNodeType<CheckEnergyActive>("CheckEnergyActive");
  factory.registerNodeType<CheckCanActivateEnergy>("CheckCanActivateEnergy");

  factory.registerNodeType<CheckHeat>("CheckHeat");
  factory.registerNodeType<CheckHeat>("CheckHeatHigh");
  factory.registerNodeType<CheckEngagedStatus>("CheckEngagedStatus");
  factory.registerNodeType<CheckEngagedStatus>("CheckEngaged");
  factory.registerNodeType<CheckOutpostTarget>("CheckOutpostTarget");
  factory.registerNodeType<CheckOutpostLowHealthDefend>("CheckOutpostLowHealthDefend");
  factory.registerNodeType<CheckEnemyAreaRecentlyHurt>("CheckEnemyAreaRecentlyHurt");
  factory.registerNodeType<CheckTargetDistance>("CheckTargetDistance");
  factory.registerNodeType<CheckCrossZoneTransition>("CheckCrossZoneTransition");
  factory.registerNodeType<CheckCapacitorCapacity>("CheckCapacitorCapacity");
  factory.registerNodeType<CheckStanceCooldown>("CheckStanceCooldown");
  factory.registerNodeType<CheckStanceEffectLimit>("CheckStanceEffectLimit");
  factory.registerNodeType<CheckStanceRefreshRequired>("CheckStanceRefreshRequired");
  factory.registerNodeType<CheckTunnelDeformation>("CheckTunnelDeformation");
  factory.registerNodeType<CheckInEnemyFortZone>("CheckInEnemyFortZone");
  factory.registerNodeType<CheckEnemyDefenseHealthDrop>("CheckEnemyDefenseHealthDrop");
  factory.registerNodeType<CheckManualStanceOverride>("CheckManualStanceOverride");
  factory.registerNodeType<CheckShouldEnhanceStance>("CheckShouldEnhanceStance");
  factory.registerNodeType<CheckTargetVisible>("CheckTargetVisible");
  factory.registerNodeType<CheckNearEnemyOutpost>("CheckNearEnemyOutpost", zones);

  // ---- actions ----
  factory.registerNodeType<SetCoordinate>("SetCoordinate", zones);
  factory.registerNodeType<SetNavMode>("SetNavMode");
  factory.registerNodeType<SetManualNavGoal>("SetManualNavGoal");
  factory.registerNodeType<SetManualNavGoal>("SetManualOverrideGoal");
  factory.registerNodeType<SetTargetCoordinate>("SetTargetCoordinate");
  factory.registerNodeType<SelectPatrolPoint>("SelectPatrolPoint", zones);
  factory.registerNodeType<Wait>("Wait");
  factory.registerNodeType<SetStairsPosition>("SetStairsPosition", zones);
  factory.registerNodeType<DescendStairsAction>("DescendStairsAction");
  factory.registerNodeType<AccumulateAmmoPurchase>("AccumulateAmmoPurchase");
  factory.registerNodeType<ChangeMapAction>("ChangeMapAction");
  factory.registerNodeType<ControlThroughTunnel>("ControlThroughTunnel");
  factory.registerNodeType<EmergencyStop>("EmergencyStop");

  factory.registerNodeType<NavigateToPoseAction>("NavigateToPose", node);
  factory.registerNodeType<NavigateToPoseAction>("NavigateToPoseAction", node);

  factory.registerNodeType<SetTacticalMode>("SetTacticalMode");
  factory.registerNodeType<ChangeTacticalAction>("ChangeTacticalAction");

  factory.registerNodeType<RequestRevive>("RequestRevive");
  factory.registerNodeType<RequestRevive>("RequestReviveAction");
  factory.registerNodeType<ClearRevive>("ClearRevive");
  factory.registerNodeType<RequestRemoteAmmoExchangeAction>("RequestRemoteAmmoExchangeAction");
  factory.registerNodeType<RequestRemoteHealthExchangeAction>("RequestRemoteHealthExchangeAction");

  factory.registerNodeType<SetGyroState>("SetGyroState");
  factory.registerNodeType<SetDesiredStance>("SetDesiredStance");
  factory.registerNodeType<SetSuperCap>("SetSuperCap");
  factory.registerNodeType<ChangeStance>("ChangeStance");
  factory.registerNodeType<TunnelGyroAlignAction>("TunnelGyroAlignAction");
  factory.registerNodeType<UpdateStanceDuration>("UpdateStanceDuration");
  factory.registerNodeType<ApplyManualStanceOverride>("ApplyManualStanceOverride");

  factory.registerNodeType<TrackTargetAction>("TrackTargetAction");
  factory.registerNodeType<SetGimbalPose>("SetGimbalPose");
  factory.registerNodeType<SetGimbalPoseByAreaAction>("SetGimbalPoseByAreaAction", zones);
  factory.registerNodeType<TunnelTimeoutBackoutAction>("TunnelTimeoutBackoutAction");
}



bool BtEngine::loadTrees_(const std::string & _tree_dir, const BT::Blackboard::Ptr & _blackboard)
{
  try {
    factory_.registerBehaviorTreeFromFile(_tree_dir + "/resource_tree.xml");
    factory_.registerBehaviorTreeFromFile(_tree_dir + "/tactical_tree.xml");
    factory_.registerBehaviorTreeFromFile(_tree_dir + "/recovery_tree.xml");
    factory_.registerBehaviorTreeFromFile(_tree_dir + "/nav_tree.xml");
    factory_.registerBehaviorTreeFromFile(_tree_dir + "/stance_tree.xml");
    factory_.registerBehaviorTreeFromFile(_tree_dir + "/gimbal_tree.xml");

    resource_tree_ = factory_.createTree("ResourceTree", _blackboard);
    tactical_tree_ = factory_.createTree("TacticalTree", _blackboard);
    nav_tree_ = factory_.createTree("MainTree", _blackboard);
    stance_tree_ = factory_.createTree("StanceTree", _blackboard);
    gimbal_tree_ = factory_.createTree("GimbalTree", _blackboard);
  } catch (const std::exception & ex) {
    throw std::runtime_error(std::string("BtEngine loadTrees failed: ") + ex.what());
  }
  return true;
}

bool BtEngine::initialize(
  const std::string & _tree_dir, const BT::Blackboard::Ptr & _blackboard, const ZoneMap * _zones,
  rclcpp::Node::SharedPtr _node)
{
  registerNodes_(_zones, std::move(_node));
  if (!loadTrees_(_tree_dir, _blackboard)) {
    return false;
  }
  resource_latch_ = std::make_unique<BtStatusLatch>(resource_tree_.rootNode());
  tactical_latch_ = std::make_unique<BtStatusLatch>(tactical_tree_.rootNode());
  nav_latch_ = std::make_unique<BtStatusLatch>(nav_tree_.rootNode());
  stance_latch_ = std::make_unique<BtStatusLatch>(stance_tree_.rootNode());
  gimbal_latch_ = std::make_unique<BtStatusLatch>(gimbal_tree_.rootNode());
  return true;
}

void BtEngine::tickOnce()
{
  resource_latch_->beginTick();
  resource_tree_.tickOnce();
  tactical_latch_->beginTick();
  tactical_tree_.tickOnce();
  nav_latch_->beginTick();
  nav_tree_.tickOnce();
  stance_latch_->beginTick();
  stance_tree_.tickOnce();
  gimbal_latch_->beginTick();
  gimbal_tree_.tickOnce();
}

}  // namespace rm_decision
