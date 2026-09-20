#include "rm_decision/bt/action/nav_action.hpp"
#include "rm_decision/domain/blackboard_keys.hpp"
#include <action_msgs/msg/goal_status.hpp>
#include <chrono>
#include <cstdint>
namespace rm_decision {

NavigateToPoseAction::NavigateToPoseAction(const std::string & n, const BT::NodeConfig & c, rclcpp::Node::SharedPtr node)
:StatefulActionNode(n,c), node_(std::move(node))
{
  client_=rclcpp_action::create_client<nav2_msgs::action::NavigateToPose>(node_,"navigate_to_pose");
}
BT::PortsList NavigateToPoseAction::providedPorts(){return {BT::InputPort<bool>("use_blackboard_goal",true,"")};}
bool NavigateToPoseAction::isDeadInMatch_() const{
  float h=1; std::uint8_t p=0;
  (void)config().blackboard->get(BbKey::kHealth,h);
  (void)config().blackboard->get(BbKey::kGameProgress,p);
  return p==4u && h<=0.f;
}
bool NavigateToPoseAction::readGoal_(geometry_msgs::msg::PoseStamped * out) const{
  bool valid=false;(void)config().blackboard->get(BbKey::kNavGoalValid,valid);
  if(!valid) return false;
  return config().blackboard->get(BbKey::kNavGoal,*out);
}
BT::NodeStatus NavigateToPoseAction::onStart(){
  if(isDeadInMatch_()) return BT::NodeStatus::FAILURE;
  if(!client_->wait_for_action_server(std::chrono::milliseconds(500))) return BT::NodeStatus::FAILURE;
  geometry_msgs::msg::PoseStamped goal_pose;
  if(!readGoal_(&goal_pose)) return BT::NodeStatus::FAILURE;
  goal_pose.header.stamp=node_->now();
  nav2_msgs::action::NavigateToPose::Goal goal; goal.pose=goal_pose;
  auto future=client_->async_send_goal(goal);
  if(rclcpp::spin_until_future_complete(node_,future,std::chrono::seconds(3))!=rclcpp::FutureReturnCode::SUCCESS)
    return BT::NodeStatus::FAILURE;
  goal_handle_=future.get();
  return goal_handle_ ? BT::NodeStatus::RUNNING : BT::NodeStatus::FAILURE;
}
BT::NodeStatus NavigateToPoseAction::onRunning(){
  if(isDeadInMatch_()){
    if(goal_handle_){(void)client_->async_cancel_goal(goal_handle_); goal_handle_.reset();}
    return BT::NodeStatus::FAILURE;
  }
  if(!goal_handle_) return BT::NodeStatus::FAILURE;
  const auto st=goal_handle_->get_status();
  if(st==action_msgs::msg::GoalStatus::STATUS_SUCCEEDED){goal_handle_.reset(); return BT::NodeStatus::SUCCESS;}
  if(st==action_msgs::msg::GoalStatus::STATUS_ABORTED||st==action_msgs::msg::GoalStatus::STATUS_CANCELED){
    goal_handle_.reset(); return BT::NodeStatus::FAILURE;
  }
  return BT::NodeStatus::RUNNING;
}
void NavigateToPoseAction::onHalted(){
  if(goal_handle_){(void)client_->async_cancel_goal(goal_handle_); goal_handle_.reset();}
}
}  // namespace rm_decision
