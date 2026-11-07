#include "rm_decision/infrastructure/auto_aim_ingress.hpp"
#include "rm_decision/domain/blackboard_keys.hpp"

#include <geometry_msgs/msg/point_stamped.hpp>
#include <tf2/exceptions.hpp>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

#include <cmath>

namespace rm_decision
{

AutoAimIngress::AutoAimIngress(rclcpp::Node & _node, const std::string & _topic)
    : node_(_node), tf_buffer_(node_.get_clock()), tf_listener_(tf_buffer_, &node_, false)
{
    sub_ = node_.create_subscription<decision_interfaces::msg::Target>(
        _topic, rclcpp::SensorDataQoS(),
        [this](const decision_interfaces::msg::Target::SharedPtr msg) { onTarget_(msg); });
    RCLCPP_INFO(node_.get_logger(), "AutoAimIngress: subscribe %s", _topic.c_str());
}


void AutoAimIngress::onTarget_(const decision_interfaces::msg::Target::SharedPtr _msg)
{
    std::lock_guard<std::mutex> lock(mutex_);
    latest_ = *_msg;
    received_at_ = std::chrono::steady_clock::now();
}


void AutoAimIngress::update(BT::Blackboard & _bb)
{
    double timeout_sec = 0.2;
    _bb.set(BbKey::kTargetValid, false);

    std::optional<decision_interfaces::msg::Target> copy;
    std::chrono::steady_clock::time_point received_at;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        copy = latest_;
        received_at = received_at_;
    }
    if (!copy || !copy->tracking) {
        return;
    }
    const double elapsed_sec =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - received_at).count();
    if (elapsed_sec > timeout_sec) {
        return;
    }

    geometry_msgs::msg::PointStamped target;
    target.header = copy->header;
    target.point = copy->position;
    geometry_msgs::msg::TransformStamped current;
    try {
        target = tf_buffer_.transform(target, "map");
        current = tf_buffer_.lookupTransform("map", "base_link", tf2::TimePointZero);
    } catch (const tf2::TransformException &) {
        return;
    }
    const double distance = std::hypot(
        target.point.x - current.transform.translation.x,
        target.point.y - current.transform.translation.y);
    _bb.set(BbKey::kTargetPoseX, target.point.x);
    _bb.set(BbKey::kTargetPoseY, target.point.y);
    _bb.set(BbKey::kTargetDistance, distance);
    _bb.set(BbKey::kTargetValid, copy->tracking);
}

}
