#include "rm_decision/bt/action/auto_actions.hpp"
#include "rm_decision/bt/bt_utils.hpp"
#include "rm_decision/domain/blackboard_keys.hpp"
#include <chrono>
#include <cstdint>
namespace rm_decision {
namespace {
double nowSec(){
  const auto now=std::chrono::steady_clock::now().time_since_epoch();
  return std::chrono::duration_cast<std::chrono::duration<double>>(now).count();
}
}

SetCoordinate::SetCoordinate(const std::string & n, const BT::NodeConfig & c, const ZoneMap * z)
:SyncActionNode(n,c),zones_(z){}
BT::PortsList SetCoordinate::providedPorts(){return {BT::InputPort<int>("goal",0,"")};}
BT::NodeStatus SetCoordinate::tick(){
  if(!zones_) return BT::NodeStatus::FAILURE;
  int id=0;(void)getInput("goal",id);
  Point2d_s pt; if(!zones_->getNavPoint(id,&pt)) return BT::NodeStatus::FAILURE;
  config().blackboard->set(BbKey::kNavGoal, poseFromXyYaw(pt.x,pt.y,pt.yaw));
  config().blackboard->set(BbKey::kNavGoalValid, true);
  return BT::NodeStatus::SUCCESS;
}

SetNavMode::SetNavMode(const std::string & n, const BT::NodeConfig & c):SyncActionNode(n,c){}
BT::PortsList SetNavMode::providedPorts(){return {BT::InputPort<std::string>("mode","patrol","")};}
BT::NodeStatus SetNavMode::tick(){
  std::string mode="patrol";(void)getInput("mode",mode);
  std::uint8_t v=static_cast<std::uint8_t>(NavMode_e::Patrol);
  if(mode=="retreat") v=static_cast<std::uint8_t>(NavMode_e::Retreat);
  else if(mode=="response") v=static_cast<std::uint8_t>(NavMode_e::Response);
  else if(mode=="manual") v=static_cast<std::uint8_t>(NavMode_e::Manual);
  config().blackboard->set(BbKey::kNavMode,v);
  return BT::NodeStatus::SUCCESS;
}

SetManualNavGoal::SetManualNavGoal(const std::string & n, const BT::NodeConfig & c):SyncActionNode(n,c){}
BT::PortsList SetManualNavGoal::providedPorts()
{
  return {
    BT::InputPort<double>("wait_duration_s", 5.0, ""),
    BT::InputPort<double>("goal_radius", 0.5, ""),
    BT::InputPort<double>("fallback_timeout_s", 60.0, ""),
  };
}
BT::NodeStatus SetManualNavGoal::tick(){
  (void)getInput<double>("wait_duration_s");
  (void)getInput<double>("goal_radius");
  (void)getInput<double>("fallback_timeout_s");
  double x=0,y=0;(void)config().blackboard->get(BbKey::kManualGoalX,x);(void)config().blackboard->get(BbKey::kManualGoalY,y);
  config().blackboard->set(BbKey::kNavGoal, poseFromXyYaw(x,y,0.0));
  config().blackboard->set(BbKey::kNavGoalValid, true);
  return BT::NodeStatus::SUCCESS;
}

SetTargetCoordinate::SetTargetCoordinate(const std::string & n, const BT::NodeConfig & c):SyncActionNode(n,c){}
BT::PortsList SetTargetCoordinate::providedPorts(){
  return {
    BT::InputPort<double>("x",0.0,""),BT::InputPort<double>("y",0.0,""),BT::InputPort<double>("yaw",0.0,""),
    BT::InputPort<double>("tracing_dist",0.0,""),
  };
}
BT::NodeStatus SetTargetCoordinate::tick(){
  double x=0,y=0,yaw=0;(void)getInput("x",x);(void)getInput("y",y);(void)getInput("yaw",yaw);
  config().blackboard->set(BbKey::kNavGoal, poseFromXyYaw(x,y,yaw));
  config().blackboard->set(BbKey::kNavGoalValid, true);
  return BT::NodeStatus::SUCCESS;
}

SelectPatrolPoint::SelectPatrolPoint(const std::string & n, const BT::NodeConfig & c, const ZoneMap * z)
:SyncActionNode(n,c),zones_(z){}
BT::PortsList SelectPatrolPoint::providedPorts(){
  return {
    BT::InputPort<int>("branch",0,""),
    BT::InputPort<int>("patrol_branch",0,""),
    BT::InputPort<int>("patrol_branch_balanced",0,""),
    BT::InputPort<int>("patrol_branch_defensive",0,""),
    BT::InputPort<int>("patrol_branch_offensive",0,""),
  };
}
BT::NodeStatus SelectPatrolPoint::tick(){
  if(!zones_ || zones_->navPoints().empty()) return BT::NodeStatus::FAILURE;
  static const int kCycle[]={0,1,2,5};
  const int id=kCycle[patrol_index_%4]; patrol_index_=(patrol_index_+1)%4;
  Point2d_s pt; if(!zones_->getNavPoint(id,&pt)) return BT::NodeStatus::FAILURE;
  config().blackboard->set(BbKey::kNavGoal, poseFromXyYaw(pt.x,pt.y,pt.yaw));
  config().blackboard->set(BbKey::kNavGoalValid, true);
  config().blackboard->set(BbKey::kNavMode, static_cast<std::uint8_t>(NavMode_e::Patrol));
  return BT::NodeStatus::SUCCESS;
}

Wait::Wait(const std::string & n, const BT::NodeConfig & c):StatefulActionNode(n,c){}
BT::PortsList Wait::providedPorts(){
  return {
    BT::InputPort<double>("duration_sec",1.0,""),
    BT::InputPort<int>("milliseconds",1000,""),
  };
}
BT::NodeStatus Wait::onStart(){
  double d=1.0;
  int ms=1000;
  if(getInput("milliseconds", ms)) d = ms / 1000.0;
  else (void)getInput("duration_sec",d);
  end_sec_=nowSec()+d; return BT::NodeStatus::RUNNING;
}
BT::NodeStatus Wait::onRunning(){ return nowSec()>=end_sec_ ? BT::NodeStatus::SUCCESS : BT::NodeStatus::RUNNING; }

SetStairsPosition::SetStairsPosition(const std::string & n, const BT::NodeConfig & c, const ZoneMap * z)
:SyncActionNode(n,c),zones_(z){}
BT::PortsList SetStairsPosition::providedPorts(){return {BT::InputPort<std::string>("zone_name","stairs","")};}
BT::NodeStatus SetStairsPosition::tick(){
  if(!zones_) return BT::NodeStatus::FAILURE;
  // Prefer nav point near stairs if zone centroid unknown: use hero_guard as approx fallback
  Point2d_s pt; if(!zones_->getNavPoint(6,&pt)) return BT::NodeStatus::FAILURE;
  config().blackboard->set(BbKey::kNavGoal, poseFromXyYaw(pt.x,pt.y,pt.yaw));
  config().blackboard->set(BbKey::kNavGoalValid, true);
  return BT::NodeStatus::SUCCESS;
}

DescendStairsAction::DescendStairsAction(const std::string & n, const BT::NodeConfig & c):StatefulActionNode(n,c){}
BT::PortsList DescendStairsAction::providedPorts(){return {BT::InputPort<double>("duration_sec",2.0,"")};}
BT::NodeStatus DescendStairsAction::onStart(){
  // TODO(bit-port): timed descend with ros clock
  return BT::NodeStatus::FAILURE;
}
BT::NodeStatus DescendStairsAction::onRunning(){ return BT::NodeStatus::FAILURE; }

AccumulateAmmoPurchase::AccumulateAmmoPurchase(const std::string & n, const BT::NodeConfig & c):SyncActionNode(n,c){}
BT::PortsList AccumulateAmmoPurchase::providedPorts(){
  return {BT::InputPort<int>("step",50,""), BT::InputPort<double>("cooldown_sec",5.0,"")};
}
BT::NodeStatus AccumulateAmmoPurchase::tick(){
  int step=50; double cd=5.0;(void)getInput("step",step);(void)getInput("cooldown_sec",cd);
  const double now=nowSec();
  if(last_purchase_sec_>0.0 && (now-last_purchase_sec_)<cd) return BT::NodeStatus::SUCCESS;
  std::uint16_t total=0;(void)config().blackboard->get(BbKey::kAmmoPurchaseTotal,total);
  total=static_cast<std::uint16_t>(total+static_cast<std::uint16_t>(step));
  config().blackboard->set(BbKey::kAmmoPurchaseTotal,total);
  last_purchase_sec_=now;
  return BT::NodeStatus::SUCCESS;
}

ChangeMapAction::ChangeMapAction(const std::string & n, const BT::NodeConfig & c):SyncActionNode(n,c){}
BT::PortsList ChangeMapAction::providedPorts(){return {BT::InputPort<std::string>("map_yaml","","")};}
BT::NodeStatus ChangeMapAction::tick(){
  // TODO(bit-port): nav2_msgs/srv/LoadMap
  return BT::NodeStatus::FAILURE;
}

ControlThroughTunnel::ControlThroughTunnel(const std::string & n, const BT::NodeConfig & c):StatefulActionNode(n,c){}
BT::PortsList ControlThroughTunnel::providedPorts(){return {};}
BT::NodeStatus ControlThroughTunnel::onStart(){
  // TODO(bit-port): controller_server params + tunnel FSM
  return BT::NodeStatus::FAILURE;
}
BT::NodeStatus ControlThroughTunnel::onRunning(){ return BT::NodeStatus::FAILURE; }

EmergencyStop::EmergencyStop(const std::string & n, const BT::NodeConfig & c):SyncActionNode(n,c){}
BT::NodeStatus EmergencyStop::tick(){
  // TODO(bit-port): preempt cmd_vel
  config().blackboard->set(BbKey::kNavGoalValid, false);
  return BT::NodeStatus::SUCCESS;
}
}  // namespace rm_decision
