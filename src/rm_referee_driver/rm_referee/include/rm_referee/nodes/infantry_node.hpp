#pragma once

#include <memory>

#include "rm_referee/referee_node.hpp"
#include "rm_referee/ros_convert.hpp"

namespace rm_referee
{

/// Commands an infantry robot subscribes to on the standard link.
using InfantryCommands = CommandList<
  cmd::GameStatus,
  cmd::GameResult,
  cmd::GameRobotHP,
  cmd::FieldEvent,
  cmd::RefereeWarning,
  cmd::RobotStatus,
  cmd::PowerHeat,
  cmd::RobotPosition,
  cmd::RobotBuff,
  // cmd::RobotHurt,
  cmd::Shoot,
  cmd::ProjectileAllowance,
  cmd::RfidStatus,
  cmd::RobotInteraction,
  cmd::MapCommand>;

/// Example subclass: publishing the referee data an infantry decision stack
/// normally consumes, plus a robot-to-robot interaction transmitter.
class InfantryRefereeNode
  : public RefereeNode<InfantryRefereeNode, InfantryCommands>
{
public:
  using Base = RefereeNode<InfantryRefereeNode, InfantryCommands>;

  explicit InfantryRefereeNode(
    const rclcpp::NodeOptions & options = rclcpp::NodeOptions())
  : Base("referee_node", options)
  {
    game_status_pub_ = create_publisher<rm_referee_msgs::msg::GameStatus>("~/game_status", 10);
    game_result_pub_ = create_publisher<rm_referee_msgs::msg::GameResult>("~/game_result", 10);
    game_robot_hp_pub_ = create_publisher<rm_referee_msgs::msg::GameRobotHP>("~/game_robot_hp", 10);
    field_event_pub_ = create_publisher<rm_referee_msgs::msg::FieldEvent>("~/field_event", 10);
    referee_warning_pub_ = create_publisher<rm_referee_msgs::msg::RefereeWarning>("~/referee_warning", 10);
    robot_status_pub_ = create_publisher<rm_referee_msgs::msg::RobotStatus>("~/robot_status", 10);
    power_heat_pub_ = create_publisher<rm_referee_msgs::msg::PowerHeat>("~/power_heat", 10);
    robot_position_pub_ = create_publisher<rm_referee_msgs::msg::RobotPosition>("~/robot_position", 50);
    robot_buff_pub_ = create_publisher<rm_referee_msgs::msg::RobotBuff>("~/robot_buff", 10);
    robot_hurt_pub_ = create_publisher<rm_referee_msgs::msg::RobotHurt>("~/robot_hurt", 10);
    shoot_pub_ = create_publisher<rm_referee_msgs::msg::Shoot>("~/shoot", 10);
    projectile_allowance_pub_ =
      create_publisher<rm_referee_msgs::msg::ProjectileAllowance>("~/projectile_allowance", 10);
    rfid_status_pub_ = create_publisher<rm_referee_msgs::msg::RfidStatus>("~/rfid_status", 10);
    robot_interaction_pub_ =
      create_publisher<rm_referee_msgs::msg::RobotInteraction>("~/robot_interaction", 10);
    map_command_pub_ = create_publisher<rm_referee_msgs::msg::MapCommand>("~/map_command", 10);

    interaction_sub_ = create_subscription<rm_referee_msgs::msg::RobotInteraction>(
      "~/tx/robot_interaction", 10,
      [this](const rm_referee_msgs::msg::RobotInteraction::SharedPtr message) {
        if (!send(convert::from_msg(*message))) {
          RCLCPP_WARN_THROTTLE(
            get_logger(), *get_clock(), 2000, "failed to send robot interaction");
        }
      });

    start();
  }

  ~InfantryRefereeNode() override
  {
    // Join the receive thread before this class's publishers are destroyed.
    stop();
  }

  void on_command(const cmd::GameStatus & value)
  {
    game_status_pub_->publish(convert::to_msg(value, make_header(now())));
  }

  void on_command(const cmd::GameResult & value)
  {
    game_result_pub_->publish(convert::to_msg(value, make_header(now())));
  }

  void on_command(const cmd::GameRobotHP & value)
  {
    game_robot_hp_pub_->publish(convert::to_msg(value, make_header(now())));
  }

  void on_command(const cmd::FieldEvent & value)
  {
    field_event_pub_->publish(convert::to_msg(value, make_header(now())));
  }

  void on_command(const cmd::RefereeWarning & value)
  {
    referee_warning_pub_->publish(convert::to_msg(value, make_header(now())));
  }

  void on_command(const cmd::RobotStatus & value)
  {
    robot_status_pub_->publish(convert::to_msg(value, make_header(now())));
  }

  void on_command(const cmd::PowerHeat & value)
  {
    power_heat_pub_->publish(convert::to_msg(value, make_header(now())));
  }

  void on_command(const cmd::RobotPosition & value)
  {
    robot_position_pub_->publish(convert::to_msg(value, make_header(now())));
  }

  void on_command(const cmd::RobotBuff & value)
  {
    robot_buff_pub_->publish(convert::to_msg(value, make_header(now())));
  }

  void on_command(const cmd::RobotHurt & value)
  {
    robot_hurt_pub_->publish(convert::to_msg(value, make_header(now())));
  }

  void on_command(const cmd::Shoot & value)
  {
    shoot_pub_->publish(convert::to_msg(value, make_header(now())));
  }

  void on_command(const cmd::ProjectileAllowance & value)
  {
    projectile_allowance_pub_->publish(convert::to_msg(value, make_header(now())));
  }

  void on_command(const cmd::RfidStatus & value)
  {
    rfid_status_pub_->publish(convert::to_msg(value, make_header(now())));
  }

  void on_command(const cmd::RobotInteraction & value)
  {
    robot_interaction_pub_->publish(convert::to_msg(value, make_header(now())));
  }

  void on_command(const cmd::MapCommand & value)
  {
    map_command_pub_->publish(convert::to_msg(value, make_header(now())));
  }

private:
  rclcpp::Publisher<rm_referee_msgs::msg::GameStatus>::SharedPtr game_status_pub_;
  rclcpp::Publisher<rm_referee_msgs::msg::GameResult>::SharedPtr game_result_pub_;
  rclcpp::Publisher<rm_referee_msgs::msg::GameRobotHP>::SharedPtr game_robot_hp_pub_;
  rclcpp::Publisher<rm_referee_msgs::msg::FieldEvent>::SharedPtr field_event_pub_;
  rclcpp::Publisher<rm_referee_msgs::msg::RefereeWarning>::SharedPtr referee_warning_pub_;
  rclcpp::Publisher<rm_referee_msgs::msg::RobotStatus>::SharedPtr robot_status_pub_;
  rclcpp::Publisher<rm_referee_msgs::msg::PowerHeat>::SharedPtr power_heat_pub_;
  rclcpp::Publisher<rm_referee_msgs::msg::RobotPosition>::SharedPtr robot_position_pub_;
  rclcpp::Publisher<rm_referee_msgs::msg::RobotBuff>::SharedPtr robot_buff_pub_;
  rclcpp::Publisher<rm_referee_msgs::msg::RobotHurt>::SharedPtr robot_hurt_pub_;
  rclcpp::Publisher<rm_referee_msgs::msg::Shoot>::SharedPtr shoot_pub_;
  rclcpp::Publisher<rm_referee_msgs::msg::ProjectileAllowance>::SharedPtr projectile_allowance_pub_;
  rclcpp::Publisher<rm_referee_msgs::msg::RfidStatus>::SharedPtr rfid_status_pub_;
  rclcpp::Publisher<rm_referee_msgs::msg::RobotInteraction>::SharedPtr robot_interaction_pub_;
  rclcpp::Publisher<rm_referee_msgs::msg::MapCommand>::SharedPtr map_command_pub_;
  rclcpp::Subscription<rm_referee_msgs::msg::RobotInteraction>::SharedPtr interaction_sub_;
};

}  // namespace rm_referee
