#pragma once

#include <behaviortree_cpp/action_node.h>
#include <decision_interfaces/msg/gimbal_pose.hpp>
#include <rclcpp/rclcpp.hpp>

#include <memory>
#include <string>

namespace rm_decision
{

class TrackTargetAction : public BT::StatefulActionNode {
public:
    TrackTargetAction(const std::string & _name, const BT::NodeConfig & _config);
    static BT::PortsList providedPorts() { return {}; }
    BT::NodeStatus onStart() override;
    BT::NodeStatus onRunning() override;
    void onHalted() override {}
};

class SetGimbalPose : public BT::SyncActionNode {
public:
    SetGimbalPose(
        const std::string & _name, const BT::NodeConfig & _config, rclcpp::Node::SharedPtr _node);
    static BT::PortsList providedPorts();
    BT::NodeStatus tick() override;

private:
    rclcpp::Node::SharedPtr node_;
    rclcpp::Publisher<decision_interfaces::msg::GimbalPose>::SharedPtr gimbal_pose_pub_;
};

class SetEnergyAim : public BT::SyncActionNode {
public:
    SetEnergyAim(const std::string & _name, const BT::NodeConfig & _config);
    static BT::PortsList providedPorts();
    BT::NodeStatus tick() override;

private:
    bool window_open_{false};
    double start_sec_{-1.0};
};

} // namespace rm_decision
