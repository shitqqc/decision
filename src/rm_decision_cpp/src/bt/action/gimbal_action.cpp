#include "rm_decision/bt/action/gimbal_action.hpp"
#include "rm_decision/domain/blackboard_keys.hpp"
namespace rm_decision {
TrackTargetAction::TrackTargetAction(const std::string & n, const BT::NodeConfig & c):StatefulActionNode(n,c){}
BT::NodeStatus TrackTargetAction::onStart(){
  bool v=false;(void)config().blackboard->get(BbKey::kTargetValid,v);
  // TODO(bit-port): gimbal egress not wired
  return v ? BT::NodeStatus::RUNNING : BT::NodeStatus::FAILURE;
}
BT::NodeStatus TrackTargetAction::onRunning(){
  bool v=false;(void)config().blackboard->get(BbKey::kTargetValid,v);
  return v ? BT::NodeStatus::RUNNING : BT::NodeStatus::FAILURE;
}
SetGimbalPose::SetGimbalPose(const std::string & n, const BT::NodeConfig & c):SyncActionNode(n,c){}
BT::PortsList SetGimbalPose::providedPorts(){
  return {
    BT::InputPort<float>("yaw",0.f,""), BT::InputPort<float>("pitch",0.f,""),
    BT::InputPort<int>("mode",1,""),
  };
}
BT::NodeStatus SetGimbalPose::tick(){
  float yaw=0,pitch=0; int mode=1;
  (void)getInput("yaw",yaw);(void)getInput("pitch",pitch);(void)getInput("mode",mode);
  config().blackboard->set(BbKey::kGimbalYawCmd, yaw);
  config().blackboard->set(BbKey::kGimbalPitchCmd, pitch);
  config().blackboard->set(BbKey::kPitchMode, mode);
  // TODO(bit-port): no gimbal hardware egress yet
  return BT::NodeStatus::SUCCESS;
}
SetGimbalPoseByAreaAction::SetGimbalPoseByAreaAction(const std::string & n, const BT::NodeConfig & c, const ZoneMap * z)
:SyncActionNode(n,c),zones_(z){}
BT::PortsList SetGimbalPoseByAreaAction::providedPorts(){return {BT::InputPort<std::string>("area","enemy_outpost","")};}
BT::NodeStatus SetGimbalPoseByAreaAction::tick(){
  (void)zones_;
  config().blackboard->set(BbKey::kPitchMode, 1);
  // TODO(bit-port): area-based scan limits
  return BT::NodeStatus::SUCCESS;
}
}  // namespace rm_decision
