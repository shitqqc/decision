#pragma once

#include "rm_decision/interface/i_command_egress.hpp"

#include <decision_interfaces/msg/gimbal_pose.hpp>
#include <decision_interfaces/msg/sentry_cmd.hpp>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/bool.hpp>
#include <std_msgs/msg/u_int8.hpp>

#include <string>

namespace rm_decision
{

class SentryCmdEgress : public ICommandEgress {
public:
    SentryCmdEgress(
        rclcpp::Node & _node, const std::string & _sentry_cmd_topic,
        const std::string & _use_spin_topic, const std::string & _energy_aim_topic);
    // TODO: 云台输出并入 egress 后，把 `const std::string & _gimbal_pose_topic` 作为第 5 个
    // 构造参数（decision_app.cpp 里已经能取到这个参数值）。

    void publish(const BT::Blackboard & _bb) override;

private:
    rclcpp::Node & node_;
    rclcpp::Publisher<decision_interfaces::msg::SentryCmd>::SharedPtr sentry_pub_;
    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr spin_pub_;
    rclcpp::Publisher<std_msgs::msg::UInt8>::SharedPtr energy_aim_pub_;
    // TODO: 云台是目前唯一绕过 ICommandEgress、由 BT 节点自己发布的输出
    // （SetGimbalPose 在 src/bt/action/gimbal_action.cpp 里持有自己的 publisher）。
    // 在此加入下面这个成员，用 _gimbal_pose_topic 初始化，并在 publish() 中统一发布，
    // 保证所有决策输出都走同一个出口。
    // rclcpp::Publisher<decision_interfaces::msg::GimbalPose>::SharedPtr gimbal_pose_pub_;
};

} // namespace rm_decision
