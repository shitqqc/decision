#pragma once

#include <behaviortree_cpp/action_node.h>
#include <rclcpp/rclcpp.hpp>
#include <tf2_ros/buffer.hpp>
#include <tf2_ros/transform_listener.hpp>

#include <string>

namespace rm_decision
{

class TrackTargetAction : public BT::StatefulActionNode {
public:
    TrackTargetAction(
        const std::string & _name, const BT::NodeConfig & _config, rclcpp::Node::SharedPtr _node);
    static BT::PortsList providedPorts();
    BT::NodeStatus onStart() override;
    BT::NodeStatus onRunning() override;
    void onHalted() override;

private:
    BT::NodeStatus updateGoal_();

    rclcpp::Node::SharedPtr node_;
    tf2_ros::Buffer tf_buffer_;
    tf2_ros::TransformListener tf_listener_;
};

class SetGimbalPose : public BT::SyncActionNode {
public:
    SetGimbalPose(const std::string & _name, const BT::NodeConfig & _config);
    static BT::PortsList providedPorts();
    BT::NodeStatus tick() override;
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
