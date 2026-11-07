#include <memory>

#include <rclcpp/rclcpp.hpp>

#include "rm_referee/nodes/infantry_node.hpp"

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<rm_referee::InfantryRefereeNode>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
