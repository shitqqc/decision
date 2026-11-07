#pragma once

#include <memory>

#include "rm_referee/referee_node.hpp"
#include "rm_referee/ros_convert.hpp"

namespace rm_referee
{

/// Commands a radar robot subscribes to on the standard link.
using RadarCommands = CommandList<
  cmd::GameStatus,
  cmd::GameRobotHP,
  cmd::FieldEvent,
  cmd::RobotStatus,
  cmd::PowerHeat,
  cmd::RadarMark,
  cmd::RadarInfo,
  cmd::MapCommand,
  cmd::RobotInteraction>;

/// Example subclass: radar decision output plus the ability to push the
/// opponent/ally map snapshot (0x0305) to the operator station.
class RadarRefereeNode
  : public RefereeNode<RadarRefereeNode, RadarCommands>
{
public:
  using Base = RefereeNode<RadarRefereeNode, RadarCommands>;

  explicit RadarRefereeNode(
    const rclcpp::NodeOptions & options = rclcpp::NodeOptions())
  : Base("referee_node", options)
  {
    game_status_pub_ = create_publisher<rm_referee_msgs::msg::GameStatus>("~/game_status", 10);
    game_robot_hp_pub_ = create_publisher<rm_referee_msgs::msg::GameRobotHP>("~/game_robot_hp", 10);
    field_event_pub_ = create_publisher<rm_referee_msgs::msg::FieldEvent>("~/field_event", 10);
    robot_status_pub_ = create_publisher<rm_referee_msgs::msg::RobotStatus>("~/robot_status", 10);
    power_heat_pub_ = create_publisher<rm_referee_msgs::msg::PowerHeat>("~/power_heat", 10);
    radar_mark_pub_ = create_publisher<rm_referee_msgs::msg::RadarMark>("~/radar_mark", 10);
    radar_info_pub_ = create_publisher<rm_referee_msgs::msg::RadarInfo>("~/radar_info", 10);
    map_command_pub_ = create_publisher<rm_referee_msgs::msg::MapCommand>("~/map_command", 10);
    robot_interaction_pub_ =
      create_publisher<rm_referee_msgs::msg::RobotInteraction>("~/robot_interaction", 10);

    interaction_sub_ = create_subscription<rm_referee_msgs::msg::RobotInteraction>(
      "~/tx/robot_interaction", 10,
      [this](const rm_referee_msgs::msg::RobotInteraction::SharedPtr message) {
        if (!send(convert::from_msg(*message))) {
          RCLCPP_WARN_THROTTLE(
            get_logger(), *get_clock(), 2000, "failed to send robot interaction");
        }
      });

    map_robot_data_sub_ = create_subscription<rm_referee_msgs::msg::MapRobotData>(
      "~/tx/map_robot_data", 10,
      [this](const rm_referee_msgs::msg::MapRobotData::SharedPtr message) {
        if (!send(convert::from_msg(*message))) {
          RCLCPP_WARN_THROTTLE(
            get_logger(), *get_clock(), 2000, "failed to send map robot data");
        }
      });

    start();
  }

  ~RadarRefereeNode() override
  {
    // Join the receive thread before this class's publishers are destroyed.
    stop();
  }

  void on_command(const cmd::GameStatus & value)
  {
    game_status_pub_->publish(convert::to_msg(value, make_header(now())));
  }

  void on_command(const cmd::GameRobotHP & value)
  {
    game_robot_hp_pub_->publish(convert::to_msg(value, make_header(now())));
  }

  void on_command(const cmd::FieldEvent & value)
  {
    field_event_pub_->publish(convert::to_msg(value, make_header(now())));
  }

  void on_command(const cmd::RobotStatus & value)
  {
    robot_status_pub_->publish(convert::to_msg(value, make_header(now())));
  }

  void on_command(const cmd::PowerHeat & value)
  {
    power_heat_pub_->publish(convert::to_msg(value, make_header(now())));
  }

  void on_command(const cmd::RadarMark & value)
  {
    radar_mark_pub_->publish(convert::to_msg(value, make_header(now())));
  }

  void on_command(const cmd::RadarInfo & value)
  {
    radar_info_pub_->publish(convert::to_msg(value, make_header(now())));
  }

  void on_command(const cmd::MapCommand & value)
  {
    map_command_pub_->publish(convert::to_msg(value, make_header(now())));
  }

  void on_command(const cmd::RobotInteraction & value)
  {
    robot_interaction_pub_->publish(convert::to_msg(value, make_header(now())));
  }

private:
  rclcpp::Publisher<rm_referee_msgs::msg::GameStatus>::SharedPtr game_status_pub_;
  rclcpp::Publisher<rm_referee_msgs::msg::GameRobotHP>::SharedPtr game_robot_hp_pub_;
  rclcpp::Publisher<rm_referee_msgs::msg::FieldEvent>::SharedPtr field_event_pub_;
  rclcpp::Publisher<rm_referee_msgs::msg::RobotStatus>::SharedPtr robot_status_pub_;
  rclcpp::Publisher<rm_referee_msgs::msg::PowerHeat>::SharedPtr power_heat_pub_;
  rclcpp::Publisher<rm_referee_msgs::msg::RadarMark>::SharedPtr radar_mark_pub_;
  rclcpp::Publisher<rm_referee_msgs::msg::RadarInfo>::SharedPtr radar_info_pub_;
  rclcpp::Publisher<rm_referee_msgs::msg::MapCommand>::SharedPtr map_command_pub_;
  rclcpp::Publisher<rm_referee_msgs::msg::RobotInteraction>::SharedPtr robot_interaction_pub_;
  rclcpp::Subscription<rm_referee_msgs::msg::RobotInteraction>::SharedPtr interaction_sub_;
  rclcpp::Subscription<rm_referee_msgs::msg::MapRobotData>::SharedPtr map_robot_data_sub_;
};

}  // namespace rm_referee
