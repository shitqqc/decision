#include <gtest/gtest.h>

#include <cstdlib>
#include <cstdint>
#include <vector>

#include <rclcpp/rclcpp.hpp>

#include "rm_referee/codec.hpp"
#include "rm_referee/commands.hpp"
#include "rm_referee/frame.hpp"
#include "rm_referee/referee_node.hpp"

namespace
{

using TestCommands = rm_referee::CommandList<
  rm_referee::cmd::GameStatus,
  rm_referee::cmd::RobotStatus,
  rm_referee::cmd::RobotInteraction>;

class TestRefereeNode : public rm_referee::RefereeNode<TestRefereeNode, TestCommands>
{
public:
  using Base = rm_referee::RefereeNode<TestRefereeNode, TestCommands>;

  TestRefereeNode()
  : Base(
      "test_referee_node",
      rclcpp::NodeOptions().parameter_overrides({
        rclcpp::Parameter("port", "/dev/null"),
        rclcpp::Parameter("publish_raw", false),
      }))
  {
  }

  void on_command(const rm_referee::cmd::GameStatus & value)
  {
    game_status = value;
    ++game_status_count;
  }

  void on_command(const rm_referee::cmd::RobotStatus & value)
  {
    robot_status = value;
    ++robot_status_count;
  }

  void on_command(const rm_referee::cmd::RobotInteraction & value)
  {
    interaction = value;
    ++interaction_count;
  }

  rm_referee::cmd::GameStatus game_status{};
  rm_referee::cmd::RobotStatus robot_status{};
  rm_referee::cmd::RobotInteraction interaction;
  int game_status_count{0};
  int robot_status_count{0};
  int interaction_count{0};
};

class RefereeNodeTest : public ::testing::Test
{
protected:
  static void SetUpTestSuite()
  {
    // Keep ROS logging out of $HOME, which may be read-only in build sandboxes.
    setenv("ROS_LOG_DIR", "/tmp/rm_referee_test_logs", 1);
    rclcpp::init(0, nullptr);
  }

  static void TearDownTestSuite()
  {
    rclcpp::shutdown();
  }
};

}  // namespace

TEST_F(RefereeNodeTest, DispatchesListedCommands)
{
  TestRefereeNode node;
  int frame_count = 0;
  node.set_frame_callback([&frame_count](const rm_referee::Frame &) {++frame_count;});

  rm_referee::cmd::GameStatus game{};
  game.game_type_and_progress = 0x13;
  game.stage_remain_time = 42;
  node.feed(rm_referee::build_frame(
      rm_referee::cmd::GameStatus::kId, rm_referee::codec::encode(game), 9));

  rm_referee::cmd::RobotStatus status{};
  status.robot_id = 7;
  status.current_hp = 123;
  node.feed(rm_referee::build_frame(
      rm_referee::cmd::RobotStatus::kId, rm_referee::codec::encode(status), 10));

  rm_referee::cmd::RobotInteraction interaction;
  interaction.data_cmd_id = 0x0200;
  interaction.sender_id = 9;
  interaction.receiver_id = 0x8080;
  interaction.user_data = {0xAA, 0xBB};
  node.feed(rm_referee::build_frame(
      rm_referee::cmd::RobotInteraction::kId, rm_referee::codec::encode(interaction), 11));

  EXPECT_EQ(frame_count, 3);
  ASSERT_EQ(node.game_status_count, 1);
  EXPECT_EQ(node.game_status.game_type(), 3U);
  EXPECT_EQ(node.game_status.stage_remain_time, 42U);
  ASSERT_EQ(node.robot_status_count, 1);
  EXPECT_EQ(node.robot_status.current_hp, 123U);
  ASSERT_EQ(node.interaction_count, 1);
  EXPECT_EQ(node.interaction.data_cmd_id, 0x0200U);
  EXPECT_EQ(node.interaction.user_data, std::vector<std::uint8_t>({0xAA, 0xBB}));
}

TEST_F(RefereeNodeTest, ParsesFrameSplitAcrossFeeds)
{
  TestRefereeNode node;

  rm_referee::cmd::GameStatus game{};
  game.stage_remain_time = 7;
  const std::vector<std::uint8_t> frame = rm_referee::build_frame(
    rm_referee::cmd::GameStatus::kId, rm_referee::codec::encode(game), 1);

  for (std::size_t i = 0; i < frame.size(); ++i) {
    node.feed(&frame[i], 1);
  }

  EXPECT_EQ(node.game_status_count, 1);
  EXPECT_EQ(node.game_status.stage_remain_time, 7U);
}

TEST_F(RefereeNodeTest, IgnoresCommandsOutsideTheList)
{
  TestRefereeNode node;
  int frame_count = 0;
  node.set_frame_callback([&frame_count](const rm_referee::Frame &) {++frame_count;});

  rm_referee::cmd::FieldEvent event{};
  event.event_data = 1;
  node.feed(rm_referee::build_frame(
      rm_referee::cmd::FieldEvent::kId, rm_referee::codec::encode(event), 3));

  EXPECT_EQ(frame_count, 1);
  EXPECT_EQ(node.game_status_count, 0);
  EXPECT_EQ(node.robot_status_count, 0);
  EXPECT_EQ(node.interaction_count, 0);
}

TEST_F(RefereeNodeTest, RejectsPayloadWithWrongLength)
{
  TestRefereeNode node;

  const std::vector<std::uint8_t> short_payload(4, 0);
  node.feed(rm_referee::build_frame(
      rm_referee::cmd::RobotStatus::kId, short_payload, 3));

  EXPECT_EQ(node.robot_status_count, 0);
}
