#include "rm_decision/bt/action/change_stance_action.hpp"
#include "rm_decision/domain/blackboard_keys.hpp"
#include <cstdint>
namespace rm_decision {
static std::uint8_t parseStance(const std::string & s){
  if(s=="ATTACK"||s=="attack") return static_cast<std::uint8_t>(StanceCmd_e::Attack);
  if(s=="DEFEND"||s=="defend") return static_cast<std::uint8_t>(StanceCmd_e::Defend);
  if(s=="ENHANCED_ATTACK") return static_cast<std::uint8_t>(StanceCmd_e::EnhancedAttack);
  if(s=="ENHANCED_DEFEND") return static_cast<std::uint8_t>(StanceCmd_e::EnhancedDefend);
  if(s=="ENHANCED_MOVE") return static_cast<std::uint8_t>(StanceCmd_e::EnhancedMove);
  return static_cast<std::uint8_t>(StanceCmd_e::Move);
}
SetGyroState::SetGyroState(const std::string & n, const BT::NodeConfig & c):SyncActionNode(n,c){}
BT::PortsList SetGyroState::providedPorts(){
  return {
    BT::InputPort<bool>("use_gyro",false,""),
    BT::InputPort<float>("gyro_vel",0.f,""),
    BT::InputPort<bool>("random_speed",false,""),
    BT::InputPort<float>("change_speed_duration",0.f,""),
  };
}
BT::NodeStatus SetGyroState::tick(){
  bool use=false;(void)getInput("use_gyro",use);
  (void)getInput<float>("gyro_vel"); (void)getInput<bool>("random_speed");
  config().blackboard->set(BbKey::kUseSpin,use);
  return BT::NodeStatus::SUCCESS;
}
SetDesiredStance::SetDesiredStance(const std::string & n, const BT::NodeConfig & c):SyncActionNode(n,c){}
BT::PortsList SetDesiredStance::providedPorts(){return {BT::InputPort<std::string>("stance","MOVE","")};}
BT::NodeStatus SetDesiredStance::tick(){ std::string s="MOVE";(void)getInput("stance",s); config().blackboard->set(BbKey::kDesiredStance, parseStance(s)); return BT::NodeStatus::SUCCESS; }
SetSuperCap::SetSuperCap(const std::string & n, const BT::NodeConfig & c):SyncActionNode(n,c){}
BT::PortsList SetSuperCap::providedPorts(){return {BT::InputPort<bool>("enable",false,"")};}
BT::NodeStatus SetSuperCap::tick(){ bool en=false;(void)getInput("enable",en); config().blackboard->set(BbKey::kUseSuperCap,en); return BT::NodeStatus::SUCCESS; }
ChangeStance::ChangeStance(const std::string & n, const BT::NodeConfig & c):StatefulActionNode(n,c){}
BT::PortsList ChangeStance::providedPorts(){return {BT::InputPort<std::string>("stance","MOVE","")};}
BT::NodeStatus ChangeStance::onStart(){
  std::string s="MOVE";(void)getInput("stance",s);
  config().blackboard->set(BbKey::kDesiredStance, parseStance(s));
  return BT::NodeStatus::RUNNING;
}
BT::NodeStatus ChangeStance::onRunning(){
  // Wait until posture feedback matches if available; else succeed immediately
  std::uint8_t desired = 3;
  std::uint8_t feedback = 0;
  (void)config().blackboard->get(BbKey::kDesiredStance, desired);
  (void)config().blackboard->get(BbKey::kPostureFeedback, feedback);
  if (feedback == 0) {
    (void)config().blackboard->get(BbKey::kCurrentStance, feedback);
  }
  if (feedback == 0 || feedback == desired) {
    config().blackboard->set(BbKey::kCurrentStance, desired);
    return BT::NodeStatus::SUCCESS;
  }
  return BT::NodeStatus::RUNNING;
}
TunnelGyroAlignAction::TunnelGyroAlignAction(const std::string & n, const BT::NodeConfig & c):SyncActionNode(n,c){}
BT::PortsList TunnelGyroAlignAction::providedPorts(){
  return {
    BT::InputPort<float>("angle_deg",0.f,""),
    BT::InputPort<bool>("reuse_existing_target",false,""),
  };
}
BT::NodeStatus TunnelGyroAlignAction::tick(){
  // TODO(bit-port): chassis tunnel yaw alignment egress
  float ang=0.f;(void)getInput("angle_deg",ang);
  config().blackboard->set(BbKey::kTunnelAlignActive, true);
  (void)ang;
  return BT::NodeStatus::FAILURE;
}
UpdateStanceDuration::UpdateStanceDuration(const std::string & n, const BT::NodeConfig & c):SyncActionNode(n,c){}
BT::NodeStatus UpdateStanceDuration::tick(){ return BT::NodeStatus::SUCCESS; }
ApplyManualStanceOverride::ApplyManualStanceOverride(const std::string & n, const BT::NodeConfig & c):SyncActionNode(n,c){}
BT::NodeStatus ApplyManualStanceOverride::tick(){
  bool active=false;(void)config().blackboard->get(BbKey::kManualStanceOverrideActive,active);
  if(!active) return BT::NodeStatus::FAILURE;
  std::uint8_t st=3;(void)config().blackboard->get(BbKey::kManualStanceOverride,st);
  config().blackboard->set(BbKey::kDesiredStance, st);
  return BT::NodeStatus::SUCCESS;
}
}  // namespace rm_decision
