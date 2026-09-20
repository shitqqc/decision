#include "rm_decision/application/decision_app.hpp"

#include <rclcpp/rclcpp.hpp>

#include <memory>

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::NodeOptions options;
  options.automatically_declare_parameters_from_overrides(true);
  auto node = std::make_shared<rclcpp::Node>("tree_exec", options);

  rm_decision::DecisionApp app(node);
  if (!app.start()) {
    RCLCPP_FATAL(node->get_logger(), "DecisionApp failed to start");
    rclcpp::shutdown();
    return 1;
  }
  app.spinLoop();
  rclcpp::shutdown();
  return 0;
}
