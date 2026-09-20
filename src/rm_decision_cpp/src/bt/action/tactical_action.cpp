#include "rm_decision/bt/action/tactical_action.hpp"
#include "rm_decision/domain/blackboard_keys.hpp"
#include <cstdint>
namespace rm_decision {
SetTacticalMode::SetTacticalMode(const std::string & n, const BT::NodeConfig & c):SyncActionNode(n,c){}
BT::PortsList SetTacticalMode::providedPorts(){return {BT::InputPort<std::string>("mode","normal","")};}
BT::NodeStatus SetTacticalMode::tick(){
  std::string mode="normal";(void)getInput("mode",mode);
  std::uint8_t v=static_cast<std::uint8_t>(TacticalMode_e::Normal);
  if(mode=="attack") v=static_cast<std::uint8_t>(TacticalMode_e::Attack);
  else if(mode=="defend") v=static_cast<std::uint8_t>(TacticalMode_e::Defend);
  config().blackboard->set(BbKey::kTacticalMode,v);
  return BT::NodeStatus::SUCCESS;
}
ChangeTacticalAction::ChangeTacticalAction(const std::string & n, const BT::NodeConfig & c):SyncActionNode(n,c){}
BT::PortsList ChangeTacticalAction::providedPorts(){return {BT::InputPort<std::string>("mode","normal","")};}
BT::NodeStatus ChangeTacticalAction::tick(){
  std::string mode="normal";(void)getInput("mode",mode);
  std::uint8_t v=static_cast<std::uint8_t>(TacticalMode_e::Normal);
  if(mode=="attack") v=static_cast<std::uint8_t>(TacticalMode_e::Attack);
  else if(mode=="defend") v=static_cast<std::uint8_t>(TacticalMode_e::Defend);
  config().blackboard->set(BbKey::kTacticalMode,v);
  return BT::NodeStatus::SUCCESS;
}
}  // namespace rm_decision
