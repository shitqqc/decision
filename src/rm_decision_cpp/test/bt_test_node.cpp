#include "rm_decision/domain/blackboard_keys.hpp"
#include "rm_decision/infrastructure/bt_status_latch.hpp"
#include "rm_decision/infrastructure/game_info_referee_ingress.hpp"
#include "rm_decision/infrastructure/odom_pose_ingress.hpp"
#include "rm_decision/infrastructure/through_tunnel_ingress.hpp"

#include <ament_index_cpp/get_package_share_directory.hpp>
#include <behaviortree_cpp/bt_factory.h>
#include <behaviortree_cpp/contrib/json.hpp>
#include <behaviortree_cpp/control_node.h>
#include <behaviortree_cpp/decorator_node.h>
#include <behaviortree_cpp/json_export.h>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>

#include <chrono>
#include <exception>
#include <filesystem>
#include <memory>
#include <string>

namespace rm_decision
{
void registerTestNodes(
  BT::BehaviorTreeFactory & factory, rclcpp::Node::SharedPtr node);
}

namespace
{
nlohmann::json serializeNode(
  const BT::TreeNode & node, const rm_decision::BtStatusLatch & latch)
{
  auto status = latch.displayStatus(node.UID());
  if (status == BT::NodeStatus::IDLE) {
    status = node.status();
  }
  nlohmann::json ports = nlohmann::json::object();
  for (const auto & port : node.config().input_ports) {
    ports[port.first] = port.second;
  }
  for (const auto & port : node.config().output_ports) {
    ports[port.first] = port.second;
  }
  nlohmann::json children = nlohmann::json::array();
  if (const auto * control = dynamic_cast<const BT::ControlNode *>(&node)) {
    for (const auto * child : control->children()) {
      children.push_back(serializeNode(*child, latch));
    }
  } else if (const auto * decorator = dynamic_cast<const BT::DecoratorNode *>(&node)) {
    if (decorator->child()) {
      children.push_back(serializeNode(*decorator->child(), latch));
    }
  }
  return {
    {"uid", node.UID()}, {"name", node.name()}, {"registration", node.registrationName()},
    {"path", node.fullPath()}, {"type", BT::toStr(node.type())},
    {"status", BT::toStr(status)}, {"ports", ports}, {"children", children}};
}

void publishSnapshot(
  rclcpp::Publisher<std_msgs::msg::String> & publisher, const BT::Tree & tree,
  const rm_decision::BtStatusLatch & latch, BT::Blackboard & blackboard)
{
  nlohmann::json values = BT::ExportBlackboardToJSON(blackboard);
  for (auto & value : values.items()) {
    if (!value.value().is_string()) {
      value.value() = value.value().dump();
    }
  }
  const auto stamp = std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now().time_since_epoch()).count();
  nlohmann::json snapshot = {
    {"stamp_ms", stamp},
    {"trees", {{"test", {{"root", serializeNode(*tree.rootNode(), latch)}}}}},
    {"blackboard", values}};
  std_msgs::msg::String message;
  message.data = snapshot.dump();
  publisher.publish(message);
}
}

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<rclcpp::Node>("bt_test");
  int result = 0;
  try {
    const auto share = ament_index_cpp::get_package_share_directory("rm_decision_cpp");
    const auto tree_directory = std::filesystem::canonical(share + "/test/tree");
    const auto requested_tree = node->declare_parameter<std::string>(
      "tree_xml_file", "test_tree.xml");
    const auto tree_path = (tree_directory / requested_tree).lexically_normal();
    const auto relative_tree = tree_path.lexically_relative(tree_directory);
    if (relative_tree.empty() || *relative_tree.begin() == ".." ||
      tree_path.extension() != ".xml" || !std::filesystem::is_regular_file(tree_path))
    {
      rclcpp::shutdown();
      return 1;
    }
    const auto tree_file = tree_path.string();

    const auto snapshot_topic = node->declare_parameter<std::string>(
      "bt_snapshot_topic", "/nav/test/bt_snapshot");
    const auto game_topic = node->declare_parameter<std::string>(
      "game_info_topic_name", "/nav/game_info");
    const auto odom_topic = node->declare_parameter<std::string>("odom_topic", "/odom");
    const auto tunnel_topic = node->declare_parameter<std::string>(
      "through_tunnel_topic", "/through_tunnel");
    const auto period = node->declare_parameter<int>("tick_period_milliseconds", 100);

    auto blackboard = BT::Blackboard::create();
    rm_decision::GameInfoRefereeIngress referee(*node, game_topic);
    rm_decision::OdomPoseIngress pose(*node, odom_topic);
    rm_decision::ThroughTunnelIngress tunnel(*node, tunnel_topic);

    BT::BehaviorTreeFactory factory;
    rm_decision::registerTestNodes(factory, node);
    auto tree = factory.createTreeFromFile(tree_file, blackboard);
    rm_decision::BtStatusLatch latch(tree.rootNode());
    auto publisher = node->create_publisher<std_msgs::msg::String>(snapshot_topic, 10);
    RCLCPP_INFO(node->get_logger(), "Test tree: %s", tree_file.c_str());
    rclcpp::WallRate rate{std::chrono::milliseconds(period)};
    try {
      while (rclcpp::ok()) {
        rclcpp::spin_some(node);
        referee.update(*blackboard);
        pose.update(*blackboard);
        tunnel.update(*blackboard);
        latch.beginTick();
        tree.tickOnce();
        publishSnapshot(*publisher, tree, latch, *blackboard);
        rate.sleep();
      }
    } catch (...) {
      tree.haltTree();
      throw;
    }
    tree.haltTree();
  } catch (const std::exception & ex) {
    RCLCPP_ERROR(node->get_logger(), "%s", ex.what());
    result = 1;
  }

  rclcpp::shutdown();
  return result;
}
