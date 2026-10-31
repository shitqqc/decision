#include "rm_decision/application/bt_engine.hpp"

#include "rm_decision/bt/action/auto_actions.hpp"
#include "rm_decision/bt/action/change_stance_action.hpp"
#include "rm_decision/bt/action/gimbal_action.hpp"
#include "rm_decision/bt/action/nav_action.hpp"
#include "rm_decision/bt/action/resource_actions.hpp"
#include "rm_decision/bt/action/tactical_action.hpp"
#include "rm_decision/bt/condition/attack_condition.hpp"
#include "rm_decision/bt/condition/auto_conditions.hpp"
#include "rm_decision/bt/condition/combat_conditions.hpp"
#include "rm_decision/bt/condition/resource_conditions.hpp"
#include "rm_decision/bt/condition/stance_conditions.hpp"
#include "rm_decision/bt/condition/target_condition.hpp"

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
  factory.registerNodeType<CheckTacticalMode>("CheckTacticalMode");
  factory.registerNodeType<CheckTacticalMode>("CheckTacticalModeCondition");
  factory.registerNodeType<CheckAmmoLow>("CheckAmmoLow");
  factory.registerNodeType<CheckOutpostRemained>("CheckOutpostRemained");
  factory.registerNodeType<CheckOwnOutpostAlive>("CheckOwnOutpostAlive");
  factory.registerNodeType<CheckRetreatCondition>("CheckRetreatCondition");
  factory.registerNodeType<UpdateOutpostAttackState>("UpdateOutpostAttackState");
  factory.registerNodeType<CheckOutpostAttackState>("CheckOutpostAttackState");
  factory.registerNodeType<CheckHeroGuardActive>("CheckHeroGuardActive");
  factory.registerNodeType<UpdateHighlandFallbackState>("UpdateHighlandFallbackState");
  factory.registerNodeType<CheckHighlandFallbackActive>("CheckHighlandFallbackActive");
  factory.registerNodeType<CheckOutpostSafeResponse>("CheckOutpostSafeResponse");
  factory.registerNodeType<CheckTargetArmorId>("CheckTargetArmorId");

  // ---- tactical / resource / stance / gimbal conditions ----
  factory.registerNodeType<CheckAttackCondition>("CheckAttackCondition");
  factory.registerNodeType<CheckHealth>("CheckHealth");
  factory.registerNodeType<CheckCoinRemaining>("CheckCoinRemaining");
  factory.registerNodeType<CheckCanFreeResurrect>("CheckCanFreeResurrect");
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
  factory.registerNodeType<CheckCapacitorCapacity>("CheckCapacitorCapacity");
  factory.registerNodeType<CheckStanceEffectLimit>("CheckStanceEffectLimit");
  factory.registerNodeType<CheckStanceRefreshRequired>("CheckStanceRefreshRequired");
  factory.registerNodeType<CheckEnemyDefenseHealthDrop>("CheckEnemyDefenseHealthDrop");
  factory.registerNodeType<CheckShouldEnhanceStance>("CheckShouldEnhanceStance");
  
  // ---- actions ----
  factory.registerNodeType<SetCoordinate>("SetCoordinate", zones);
  factory.registerNodeType<SetNavMode>("SetNavMode");
  factory.registerNodeType<SetManualNavGoal>("SetManualNavGoal");
  factory.registerNodeType<SetManualNavGoal>("SetManualOverrideGoal");
  factory.registerNodeType<SelectPatrolPoint>("SelectPatrolPoint", zones);
  factory.registerNodeType<Wait>("Wait");
  factory.registerNodeType<AccumulateAmmoPurchase>("AccumulateAmmoPurchase");

  factory.registerNodeType<NavigateToPoseAction>("NavigateToPose", node, zones);
  factory.registerNodeType<NavigateToPoseAction>("NavigateToPoseAction", node, zones);

  factory.registerNodeType<SetTacticalMode>("SetTacticalMode");

  factory.registerNodeType<RequestRevive>("RequestRevive");
  factory.registerNodeType<RequestRevive>("RequestReviveAction");
  factory.registerNodeType<ClearRevive>("ClearRevive");
  factory.registerNodeType<RequestRemoteAmmoExchangeAction>("RequestRemoteAmmoExchangeAction");
  factory.registerNodeType<RequestRemoteHealthExchangeAction>("RequestRemoteHealthExchangeAction");

  factory.registerNodeType<SetSpinState>("SetSpinState");
  factory.registerNodeType<SetDesiredStance>("SetDesiredStance");
  factory.registerNodeType<SetSuperCap>("SetSuperCap");
  factory.registerNodeType<ChangeStance>("ChangeStance");
  factory.registerNodeType<UpdateStanceDuration>("UpdateStanceDuration");
  factory.registerNodeType<ApplyManualStanceOverride>("ApplyManualStanceOverride");

  factory.registerNodeType<TrackTargetAction>("TrackTargetAction");
  factory.registerNodeType<SetGimbalPose>("SetGimbalPose", node);
  factory.registerNodeType<SetEnergyAim>("SetEnergyAim");
}



bool BtEngine::loadTrees_(const std::string & _tree_dir, const BT::Blackboard::Ptr & _blackboard)
{
  // TODO: ul和uc的树应该是
  //   match_type = rmuc -> resource / tactical / nav / stance / gimbal 五棵树
  //   match_type = rmul -> 只跑 ul_tree，其余五棵完全不 tick
  // 加 match_type 参数（见 config/node_params.yaml 的 TODO），按值只注册和只创建
  try {
    factory_.registerBehaviorTreeFromFile(_tree_dir + "/resource_tree.xml");
    factory_.registerBehaviorTreeFromFile(_tree_dir + "/tactical_tree.xml");
    factory_.registerBehaviorTreeFromFile(_tree_dir + "/nav_tree.xml");
    factory_.registerBehaviorTreeFromFile(_tree_dir + "/stance_tree.xml");
    factory_.registerBehaviorTreeFromFile(_tree_dir + "/gimbal_tree.xml");
    factory_.registerBehaviorTreeFromFile(_tree_dir + "/ul_tree.xml");

    resource_tree_ = factory_.createTree("ResourceTree", _blackboard);
    tactical_tree_ = factory_.createTree("TacticalTree", _blackboard);
    nav_tree_ = factory_.createTree("MainTree", _blackboard);
    stance_tree_ = factory_.createTree("StanceTree", _blackboard);
    gimbal_tree_ = factory_.createTree("GimbalTree", _blackboard);
    ul_tree_ = factory_.createTree("ULTree", _blackboard);
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
  ul_latch_ = std::make_unique<BtStatusLatch>(ul_tree_.rootNode());
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
  ul_latch_->beginTick();
  ul_tree_.tickOnce();
}

}  // namespace rm_decision
