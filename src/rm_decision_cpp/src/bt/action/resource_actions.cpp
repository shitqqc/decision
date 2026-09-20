#include "rm_decision/bt/action/resource_actions.hpp"
#include "rm_decision/domain/blackboard_keys.hpp"
#include <cstdint>
namespace rm_decision {
RequestRevive::RequestRevive(const std::string & n, const BT::NodeConfig & c):SyncActionNode(n,c){}
BT::PortsList RequestRevive::providedPorts()
{
  return {
    BT::InputPort<std::string>("revive_type", "free", ""),
    BT::InputPort<int>("max_instant_revive_count", 1, ""),
  };
}
BT::NodeStatus RequestRevive::tick()
{
  (void)getInput<std::string>("revive_type");
  (void)getInput<int>("max_instant_revive_count");
  config().blackboard->set(BbKey::kReviveRequest, true);
  return BT::NodeStatus::SUCCESS;
}
ClearRevive::ClearRevive(const std::string & n, const BT::NodeConfig & c):SyncActionNode(n,c){}
BT::NodeStatus ClearRevive::tick(){ config().blackboard->set(BbKey::kReviveRequest,false); return BT::NodeStatus::SUCCESS; }
RequestRemoteAmmoExchangeAction::RequestRemoteAmmoExchangeAction(const std::string & n, const BT::NodeConfig & c):SyncActionNode(n,c){}
BT::PortsList RequestRemoteAmmoExchangeAction::providedPorts(){return {BT::InputPort<int>("times",1,"")};}
BT::NodeStatus RequestRemoteAmmoExchangeAction::tick(){
  int times=1;(void)getInput("times",times);
  std::uint16_t cur=0;(void)config().blackboard->get(BbKey::kBuyProjectileTimes,cur);
  config().blackboard->set(BbKey::kBuyProjectileTimes, static_cast<std::uint16_t>(cur+static_cast<std::uint16_t>(times)));
  int cnt=0;(void)config().blackboard->get(BbKey::kRemoteAmmoExchangeCount,cnt);
  config().blackboard->set(BbKey::kRemoteAmmoExchangeCount, cnt+1);
  return BT::NodeStatus::SUCCESS;
}
RequestRemoteHealthExchangeAction::RequestRemoteHealthExchangeAction(const std::string & n, const BT::NodeConfig & c):SyncActionNode(n,c){}
BT::PortsList RequestRemoteHealthExchangeAction::providedPorts(){return {BT::InputPort<int>("times",1,"")};}
BT::NodeStatus RequestRemoteHealthExchangeAction::tick(){
  int times=1;(void)getInput("times",times);
  std::uint16_t cur=0;(void)config().blackboard->get(BbKey::kBuyHpTimes,cur);
  config().blackboard->set(BbKey::kBuyHpTimes, static_cast<std::uint16_t>(cur+static_cast<std::uint16_t>(times)));
  int cnt=0;(void)config().blackboard->get(BbKey::kRemoteHealthExchangeCount,cnt);
  config().blackboard->set(BbKey::kRemoteHealthExchangeCount, cnt+1);
  return BT::NodeStatus::SUCCESS;
}
}  // namespace rm_decision
