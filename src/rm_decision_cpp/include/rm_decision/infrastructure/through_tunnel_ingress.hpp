#pragma once

#include "rm_decision/interface/i_nav_info_ingress.hpp"

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/bool.hpp>

#include <mutex>
#include <optional>
#include <string>

namespace rm_decision
{

class ThroughTunnelIngress : public INavInfoIngress
{
public:
  ThroughTunnelIngress(rclcpp::Node & _node, const std::string & _through_tunnel_topic);

  void update(BT::Blackboard & _bb) override;

private:
  rclcpp::Node & node_;
  rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr sub_;
  mutable std::mutex mutex_;
  std::optional<std_msgs::msg::Bool> latest_;
};

}
