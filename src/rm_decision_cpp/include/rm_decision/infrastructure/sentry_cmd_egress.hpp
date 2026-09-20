#pragma once

#include "rm_decision/interface/i_command_egress.hpp"

#include <decision_interfaces/msg/sentry_cmd.hpp>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/bool.hpp>

#include <string>

namespace rm_decision
{

class SentryCmdEgress : public ICommandEgress
{
public:
  SentryCmdEgress(
    rclcpp::Node & _node, const std::string & _sentry_cmd_topic, const std::string & _use_spin_topic);

  void publish(const BT::Blackboard & _bb) override;

private:
  rclcpp::Node & node_;
  rclcpp::Publisher<decision_interfaces::msg::SentryCmd>::SharedPtr sentry_pub_;
  rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr spin_pub_;
};

}  // namespace rm_decision
