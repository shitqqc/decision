#pragma once

#include "rm_decision/interface/i_referee_ingress.hpp"

#include <decision_interfaces/msg/game_info.hpp>
#include <rclcpp/rclcpp.hpp>

#include <mutex>
#include <optional>
#include <string>

namespace rm_decision
{

class GameInfoRefereeIngress : public IRefereeIngress
{
public:
  GameInfoRefereeIngress(rclcpp::Node & _node, const std::string & _topic);

  void update(BT::Blackboard & _bb) override;

private:
  void onGameInfo_(const decision_interfaces::msg::GameInfo::SharedPtr _msg);

  rclcpp::Node & node_;
  rclcpp::Subscription<decision_interfaces::msg::GameInfo>::SharedPtr sub_;
  mutable std::mutex mutex_;
  std::optional<decision_interfaces::msg::GameInfo> latest_;
};

}  // namespace rm_decision
