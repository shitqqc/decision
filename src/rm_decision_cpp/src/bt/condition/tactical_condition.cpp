#include "rm_decision/bt/condition/tactical_condition.hpp"
#include "rm_decision/domain/blackboard_keys.hpp"
#include <cstdint>
namespace rm_decision {
CheckDefendCondition::CheckDefendCondition(const std::string & n, const BT::NodeConfig & c):ConditionNode(n,c){}
BT::PortsList CheckDefendCondition::providedPorts(){return {BT::InputPort<float>("home_hp_threshold",300.f,"")};}
BT::NodeStatus CheckDefendCondition::tick(){
  float thr=300.f;(void)getInput("home_hp_threshold",thr);
  float base=0;(void)config().blackboard->get(BbKey::kAllyBaseHp,base);
  return base>0.f && base<thr ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}
CheckAttackCondition::CheckAttackCondition(const std::string & n, const BT::NodeConfig & c, const ZoneMap * z)
:ConditionNode(n,c),zones_(z){}
BT::NodeStatus CheckAttackCondition::tick(){
  std::uint8_t mode=0;(void)config().blackboard->get(BbKey::kControlMode,mode);
  if(mode==static_cast<std::uint8_t>(ControlMode_e::Manual) && zones_){
    double x=0,y=0;(void)config().blackboard->get(BbKey::kManualGoalX,x);(void)config().blackboard->get(BbKey::kManualGoalY,y);
    if(zones_->inZone("enemy_fort",x,y)) return BT::NodeStatus::SUCCESS;
  }
  std::uint8_t st=0;(void)config().blackboard->get(BbKey::kBigEnergyStatus,st);
  int t=0;(void)config().blackboard->get(BbKey::kGameTimeRemaining,t);
  std::uint8_t progress=0;(void)config().blackboard->get(BbKey::kGameProgress,progress);
  if(progress==4u && st!=0 && t<=120) return BT::NodeStatus::SUCCESS;
  return BT::NodeStatus::FAILURE;
}
}  // namespace rm_decision
