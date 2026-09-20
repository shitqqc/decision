#include "rm_decision/bt/condition/gimbal_condition.hpp"
#include "rm_decision/domain/blackboard_keys.hpp"
#include <cmath>
namespace rm_decision {
CheckTargetVisible::CheckTargetVisible(const std::string & n, const BT::NodeConfig & c):ConditionNode(n,c){}
BT::NodeStatus CheckTargetVisible::tick(){
  bool v=false;(void)config().blackboard->get(BbKey::kTargetValid,v);
  return v ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}
CheckNearEnemyOutpost::CheckNearEnemyOutpost(const std::string & n, const BT::NodeConfig & c, const ZoneMap * z)
:ConditionNode(n,c),zones_(z){}
BT::PortsList CheckNearEnemyOutpost::providedPorts(){return {BT::InputPort<double>("radius",3.0,"")};}
BT::NodeStatus CheckNearEnemyOutpost::tick(){
  if(!zones_) return BT::NodeStatus::FAILURE;
  double r=3.0;(void)getInput("radius",r);
  bool pv=false; double x=0,y=0;
  (void)config().blackboard->get(BbKey::kPoseValid,pv);
  (void)config().blackboard->get(BbKey::kCurrentPoseX,x);
  (void)config().blackboard->get(BbKey::kCurrentPoseY,y);
  if(!pv) return BT::NodeStatus::FAILURE;
  Point2d_s pt; if(!zones_->getNavPoint(2,&pt)) return BT::NodeStatus::FAILURE;
  return std::hypot(x-pt.x,y-pt.y)<=r ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}
}  // namespace rm_decision
