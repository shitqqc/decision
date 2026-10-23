#include "rm_decision/infrastructure/through_tunnel_ingress.hpp"

#include "rm_decision/domain/blackboard_keys.hpp"

namespace rm_decision
{

ThroughTunnelIngress::ThroughTunnelIngress(
  rclcpp::Node & _node, const std::string & _through_tunnel_topic)
: node_(_node)
{
  sub_ = node_.create_subscription<std_msgs::msg::Bool>(
    _through_tunnel_topic, rclcpp::QoS(10),
    [this](const std_msgs::msg::Bool::SharedPtr msg) {
      std::lock_guard<std::mutex> lock(mutex_);
      latest_ = *msg;
    });
  RCLCPP_INFO(
    node_.get_logger(), "ThroughTunnelIngress: subscribe %s", _through_tunnel_topic.c_str());
}


void ThroughTunnelIngress::update(BT::Blackboard & _bb)
{
  std::optional<std_msgs::msg::Bool> copy;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    copy = latest_;
  }
  if (!copy) {
    _bb.set(BbKey::kThroughTunnel, false);
    return;
  }
  _bb.set(BbKey::kThroughTunnel, copy->data);
}

}
