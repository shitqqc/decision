#include "rm_decision/bt/action/nav_action.hpp"

#include "rm_decision/bt/bt_utils.hpp"
#include "rm_decision/domain/blackboard_keys.hpp"
#include "rm_decision/domain/zone_map.hpp"

#include <action_msgs/msg/goal_status.hpp>

#include <chrono>
#include <cstdint>
#include <string>

namespace rm_decision
{
namespace
{

bool parseViaIds(const std::string & _text, std::vector<int> * _ids)
{
    _ids->clear();
    std::string token;
    auto flush = [&]() -> bool {
        if (token.empty()) {
            return true;
        }
        try {
            std::size_t used = 0;
            const int id = std::stoi(token, &used);
            if (used != token.size()) {
                return false;
            }
            _ids->push_back(id);
        } catch (const std::exception &) {
            return false;
        }
        token.clear();
        return true;
    };
    for (const char ch : _text) {
        if (ch == ';' || ch == ',' || ch == ' ' || ch == '\t') {
            if (!flush()) {
                return false;
            }
        } else {
            token.push_back(ch);
        }
    }
    return flush();
}

template <typename HandleT>
BT::NodeStatus statusOf(HandleT & _handle)
{
    if (!_handle) {
        return BT::NodeStatus::FAILURE;
    }
    const auto st = _handle->get_status();
    if (st == action_msgs::msg::GoalStatus::STATUS_SUCCEEDED) {
        _handle.reset();
        return BT::NodeStatus::SUCCESS;
    }
    if (
        st == action_msgs::msg::GoalStatus::STATUS_ABORTED ||
        st == action_msgs::msg::GoalStatus::STATUS_CANCELED)
    {
        _handle.reset();
        return BT::NodeStatus::FAILURE;
    }
    return BT::NodeStatus::RUNNING;
}

}  // namespace

NavigateToPoseAction::NavigateToPoseAction(
    const std::string & n, const BT::NodeConfig & c, rclcpp::Node::SharedPtr node, const ZoneMap * zones)
    : StatefulActionNode(n, c), node_(std::move(node)), zones_(zones)
{
    client_ =
        rclcpp_action::create_client<nav2_msgs::action::NavigateToPose>(node_, "navigate_to_pose");
    through_client_ = rclcpp_action::create_client<nav2_msgs::action::NavigateThroughPoses>(
        node_, "navigate_through_poses");
}
BT::PortsList NavigateToPoseAction::providedPorts()
{
    return {
        BT::InputPort<bool>("use_blackboard_goal", true, ""),
        BT::InputPort<std::string>("via", "", "途经航点序号，分号分隔，例如 1;6。留空则只去终点"),
    };
}
bool NavigateToPoseAction::isDeadInMatch_() const
{
    float h = 1;
    std::uint8_t p = 0;
    (void)config().blackboard->get(BbKey::kHealth, h);
    (void)config().blackboard->get(BbKey::kGameProgress, p);
    return p == 4u && h <= 0.f;
}
bool NavigateToPoseAction::readGoal_(geometry_msgs::msg::PoseStamped * out) const
{
    bool valid = false;
    (void)config().blackboard->get(BbKey::kNavGoalValid, valid);
    if (!valid) {
        return false;
    }
    return config().blackboard->get(BbKey::kNavGoal, *out);
}
bool NavigateToPoseAction::readViaPoses_(std::vector<geometry_msgs::msg::PoseStamped> * out) const
{
    out->clear();
    std::string via;
    (void)getInput("via", via);
    if (via.empty()) {
        return true;
    }
    if (zones_ == nullptr) {
        return false;
    }
    std::vector<int> ids;
    if (!parseViaIds(via, &ids)) {
        return false;
    }
    out->reserve(ids.size());
    for (const int id : ids) {
        Point2d_s pt;
        if (!zones_->getNavPoint(id, &pt)) {
            RCLCPP_WARN(node_->get_logger(), "NavigateToPose via point %d is not in zones", id);
            return false;
        }
        out->push_back(poseFromXyYaw(pt.x, pt.y, pt.yaw));
    }
    return true;
}
void NavigateToPoseAction::cancelActive_()
{
    if (goal_handle_) {
        (void)client_->async_cancel_goal(goal_handle_);
        goal_handle_.reset();
    }
    if (through_handle_) {
        (void)through_client_->async_cancel_goal(through_handle_);
        through_handle_.reset();
    }
}
BT::NodeStatus NavigateToPoseAction::onStart()
{
    if (isDeadInMatch_()) {
        return BT::NodeStatus::FAILURE;
    }
    geometry_msgs::msg::PoseStamped goal_pose;
    if (!readGoal_(&goal_pose)) {
        return BT::NodeStatus::FAILURE;
    }
    std::vector<geometry_msgs::msg::PoseStamped> vias;
    if (!readViaPoses_(&vias)) {
        return BT::NodeStatus::FAILURE;
    }
    const auto stamp = node_->now();
    goal_pose.header.stamp = stamp;

    if (vias.empty()) {
        if (!client_->wait_for_action_server(std::chrono::milliseconds(500))) {
            return BT::NodeStatus::FAILURE;
        }
        nav2_msgs::action::NavigateToPose::Goal goal;
        goal.pose = goal_pose;
        auto future = client_->async_send_goal(goal);
        if (
            rclcpp::spin_until_future_complete(node_, future, std::chrono::seconds(3)) !=
            rclcpp::FutureReturnCode::SUCCESS)
        {
            return BT::NodeStatus::FAILURE;
        }
        goal_handle_ = future.get();
        return goal_handle_ ? BT::NodeStatus::RUNNING : BT::NodeStatus::FAILURE;
    }

    if (!through_client_->wait_for_action_server(std::chrono::milliseconds(500))) {
        return BT::NodeStatus::FAILURE;
    }
    nav2_msgs::action::NavigateThroughPoses::Goal goal;
    goal.poses.reserve(vias.size() + 1);
    for (auto & via : vias) {
        via.header.stamp = stamp;
        goal.poses.push_back(via);
    }
    goal.poses.push_back(goal_pose);
    auto future = through_client_->async_send_goal(goal);
    if (
        rclcpp::spin_until_future_complete(node_, future, std::chrono::seconds(3)) !=
        rclcpp::FutureReturnCode::SUCCESS)
    {
        return BT::NodeStatus::FAILURE;
    }
    through_handle_ = future.get();
    return through_handle_ ? BT::NodeStatus::RUNNING : BT::NodeStatus::FAILURE;
}
BT::NodeStatus NavigateToPoseAction::onRunning()
{
    if (isDeadInMatch_()) {
        cancelActive_();
        return BT::NodeStatus::FAILURE;
    }
    if (through_handle_) {
        return statusOf(through_handle_);
    }
    if (goal_handle_) {
        return statusOf(goal_handle_);
    }
    return BT::NodeStatus::FAILURE;
}
void NavigateToPoseAction::onHalted()
{
    cancelActive_();
}
}  // namespace rm_decision
