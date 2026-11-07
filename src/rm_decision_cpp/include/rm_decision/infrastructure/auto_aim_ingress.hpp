#pragma once

#include "rm_decision/interface/i_auto_aim_ingress.hpp"

#include <decision_interfaces/msg/target.hpp>
#include <rclcpp/rclcpp.hpp>
#include <tf2_ros/buffer.hpp>
#include <tf2_ros/transform_listener.hpp>

#include <chrono>
#include <mutex>
#include <optional>
#include <string>

namespace rm_decision
{

class AutoAimIngress : public IAutoAimIngress {
public:
    AutoAimIngress(rclcpp::Node & _node, const std::string & _topic);

    void update(BT::Blackboard & _bb) override;

private:
    void onTarget_(const decision_interfaces::msg::Target::SharedPtr _msg);

    rclcpp::Node & node_;
    tf2_ros::Buffer tf_buffer_;
    tf2_ros::TransformListener tf_listener_;
    rclcpp::Subscription<decision_interfaces::msg::Target>::SharedPtr sub_;
    mutable std::mutex mutex_;
    std::optional<decision_interfaces::msg::Target> latest_;
    std::chrono::steady_clock::time_point received_at_;
};

}
