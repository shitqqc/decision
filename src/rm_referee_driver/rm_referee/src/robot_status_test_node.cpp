#include <cinttypes>
#include <memory>

#include <rclcpp/rclcpp.hpp>

#include "rm_referee/referee_node.hpp"
#include "rm_referee/ros_convert.hpp"

namespace rm_referee
{

/// Minimal diagnostic node: it only subscribes to 0x0201 robot_status, prints
/// every decoded status to the terminal, and republishes it as a ROS message.
class RobotStatusTestNode
  : public RefereeNode<RobotStatusTestNode, CommandList<cmd::RobotStatus>>
{
public:
  RobotStatusTestNode()
  : RefereeNode("robot_status_test_node")
  {
    status_pub_ = create_publisher<rm_referee_msgs::msg::RobotStatus>(
      "~/robot_status", rclcpp::QoS(10));

    RCLCPP_INFO(
      get_logger(), "waiting for 0x0201 robot_status on %s @ %u",
      port().c_str(), baudrate());
    start();
  }

  ~RobotStatusTestNode() override
  {
    stop();
  }

  void on_command(const cmd::RobotStatus & status)
  {
    const auto message = convert::to_msg(status, make_header(now()));
    status_pub_->publish(message);

    RCLCPP_INFO(
      get_logger(),
      "0x0201 robot_status: id=%u level=%u hp=%u/%u cooling=%u heat_limit=%u "
      "chassis_power_limit=%u bullet_speed_limit=%.2f "
      "outputs[gimbal=%d chassis=%d shooter=%d]",
      status.robot_id, status.robot_level, status.current_hp, status.maximum_hp,
      status.shooter_barrel_cooling_value, status.shooter_barrel_heat_limit,
      status.chassis_power_limit, status.bullet_speed_limit,
      static_cast<int>(message.gimbal_output),
      static_cast<int>(message.chassis_output),
      static_cast<int>(message.shooter_output));
  }

private:
  rclcpp::Publisher<rm_referee_msgs::msg::RobotStatus>::SharedPtr status_pub_;
};

}  // namespace rm_referee

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<rm_referee::RobotStatusTestNode>());
  rclcpp::shutdown();
  return 0;
}
