#include "rm_decision/infrastructure/bt_snapshot_publisher.hpp"

#include <behaviortree_cpp/behavior_tree.h>
#include <behaviortree_cpp/contrib/json.hpp>
#include <behaviortree_cpp/control_node.h>
#include <behaviortree_cpp/decorator_node.h>
#include <behaviortree_cpp/json_export.h>
#include <behaviortree_cpp/utils/demangle_util.h>

#include <chrono>
#include <sstream>
#include <string>

namespace rm_decision
{

namespace
{

std::string statusToStr(BT::NodeStatus _s)
{
  switch (_s) {
    case BT::NodeStatus::IDLE:
      return "IDLE";
    case BT::NodeStatus::RUNNING:
      return "RUNNING";
    case BT::NodeStatus::SUCCESS:
      return "SUCCESS";
    case BT::NodeStatus::FAILURE:
      return "FAILURE";
    case BT::NodeStatus::SKIPPED:
      return "SKIPPED";
    default:
      return "UNKNOWN";
  }
}


nlohmann::json portsJson(const BT::TreeNode & _node)
{
  nlohmann::json ports = nlohmann::json::object();
  for (const auto & kv : _node.config().input_ports) {
    ports[kv.first] = kv.second;
  }
  for (const auto & kv : _node.config().output_ports) {
    ports[kv.first] = kv.second;
  }
  return ports;
}


nlohmann::json serializeNode(const BT::TreeNode * _node, const BtStatusLatch & _latch)
{
  nlohmann::json j;
  if (_node == nullptr) {
    return j;
  }
  j["uid"] = _node->UID();
  j["name"] = _node->name();
  j["registration"] = _node->registrationName();
  j["path"] = _node->fullPath();
  j["type"] = BT::toStr(_node->type());
  // Prefer latched non-IDLE from this tick; fall back to live status (e.g. still RUNNING).
  const BT::NodeStatus latched = _latch.displayStatus(_node->UID());
  const BT::NodeStatus live = _node->status();
  const BT::NodeStatus shown =
    (latched != BT::NodeStatus::IDLE) ? latched
    : (live != BT::NodeStatus::IDLE)  ? live
                                      : BT::NodeStatus::IDLE;
  j["status"] = statusToStr(shown);
  j["ports"] = portsJson(*_node);

  nlohmann::json children = nlohmann::json::array();
  if (auto * control = dynamic_cast<const BT::ControlNode *>(_node)) {
    for (const BT::TreeNode * child : control->children()) {
      children.push_back(serializeNode(child, _latch));
    }
  } else if (auto * deco = dynamic_cast<const BT::DecoratorNode *>(_node)) {
    if (deco->child() != nullptr) {
      children.push_back(serializeNode(deco->child(), _latch));
    }
  }
  j["children"] = std::move(children);
  return j;
}


nlohmann::json serializeTree(const BT::Tree & _tree, const BtStatusLatch & _latch)
{
  nlohmann::json t;
  t["root"] = serializeNode(_tree.rootNode(), _latch);
  return t;
}


nlohmann::json serializeBlackboard(BT::Blackboard & _bb)
{
  nlohmann::json dest = BT::ExportBlackboardToJSON(_bb);
  nlohmann::json out = nlohmann::json::object();
  for (auto it = dest.begin(); it != dest.end(); ++it) {
    if (it.value().is_string()) {
      out[it.key()] = it.value().get<std::string>();
    } else if (it.value().is_null()) {
      out[it.key()] = "null";
    } else {
      out[it.key()] = it.value().dump();
    }
  }

  for (const auto & key_view : _bb.getKeys()) {
    const std::string key(key_view);
    if (out.contains(key)) {
      continue;
    }
    if (auto any_ref = _bb.getAnyLocked(key)) {
      if (auto * any_ptr = any_ref.get()) {
        if (any_ptr->empty()) {
          out[key] = "";
        } else if (any_ptr->isString()) {
          out[key] = any_ptr->cast<std::string>();
        } else {
          std::ostringstream oss;
          oss << "<" << BT::demangle(any_ptr->type()) << ">";
          out[key] = oss.str();
        }
      }
    }
  }
  return out;
}

}  // namespace


BtSnapshotPublisher::BtSnapshotPublisher(rclcpp::Node & _node, const std::string & _topic)
: node_(_node)
{
  pub_ = node_.create_publisher<std_msgs::msg::String>(_topic, rclcpp::QoS(10).reliable());
  RCLCPP_INFO(node_.get_logger(), "BtSnapshotPublisher: topic=%s", _topic.c_str());
}


void BtSnapshotPublisher::publish(
  const BT::Tree & _resource, const BtStatusLatch & _resource_latch, const BT::Tree & _tactical,
  const BtStatusLatch & _tactical_latch, const BT::Tree & _nav, const BtStatusLatch & _nav_latch,
  const BT::Tree & _stance, const BtStatusLatch & _stance_latch, const BT::Tree & _gimbal,
  const BtStatusLatch & _gimbal_latch, BT::Blackboard & _bb)
{
  const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch())
                        .count();

  nlohmann::json root;
  root["stamp_ms"] = now_ms;
  root["trees"]["resource"] = serializeTree(_resource, _resource_latch);
  root["trees"]["tactical"] = serializeTree(_tactical, _tactical_latch);
  root["trees"]["nav"] = serializeTree(_nav, _nav_latch);
  root["trees"]["stance"] = serializeTree(_stance, _stance_latch);
  root["trees"]["gimbal"] = serializeTree(_gimbal, _gimbal_latch);
  root["blackboard"] = serializeBlackboard(_bb);

  std_msgs::msg::String msg;
  msg.data = root.dump();
  pub_->publish(msg);
}

}  // namespace rm_decision
