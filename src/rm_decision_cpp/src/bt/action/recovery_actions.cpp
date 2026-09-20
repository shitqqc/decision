#include "rm_decision/bt/action/recovery_actions.hpp"
#include "rm_decision/domain/blackboard_keys.hpp"
namespace rm_decision {
TunnelTimeoutBackoutAction::TunnelTimeoutBackoutAction(const std::string & n, const BT::NodeConfig & c):StatefulActionNode(n,c){}
BT::PortsList TunnelTimeoutBackoutAction::providedPorts(){
  return {BT::InputPort<double>("timeout_s",6.0,""), BT::InputPort<double>("max_backout_s",12.0,"")};
}
BT::NodeStatus TunnelTimeoutBackoutAction::onStart(){
  // TODO(bit-port): tunnel escape cmd_vel
  config().blackboard->set(BbKey::kTunnelEscapeActive, true);
  return BT::NodeStatus::FAILURE;
}
BT::NodeStatus TunnelTimeoutBackoutAction::onRunning(){ return BT::NodeStatus::FAILURE; }
}  // namespace rm_decision
