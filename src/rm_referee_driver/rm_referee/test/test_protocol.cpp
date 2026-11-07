#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <vector>

#include "rm_referee/codec.hpp"
#include "rm_referee/commands.hpp"
#include "rm_referee/crc.hpp"
#include "rm_referee/frame.hpp"

namespace
{

std::string to_hex(const std::vector<std::uint8_t> & bytes)
{
  static const char digits[] = "0123456789abcdef";
  std::string out;
  out.reserve(bytes.size() * 2);
  for (const std::uint8_t byte : bytes) {
    out.push_back(digits[byte >> 4U]);
    out.push_back(digits[byte & 0x0FU]);
  }
  return out;
}

std::vector<std::uint8_t> make_reference_frame()
{
  return rm_referee::build_frame(0x0001, std::vector<std::uint8_t>(11, 0), 1);
}

}  // namespace

TEST(Crc, MatchesProtocolReferenceFrame)
{
  const std::uint8_t header[4] = {0xA5, 0x0B, 0x00, 0x01};
  EXPECT_EQ(rm_referee::crc8(header, sizeof(header)), 0x5C);

  const std::vector<std::uint8_t> frame = make_reference_frame();
  ASSERT_EQ(frame.size(), 20U);
  // The tail is little-endian on the wire: bytes 66 CB encode the value 0xCB66.
  EXPECT_EQ(frame[frame.size() - 2], 0x66U);
  EXPECT_EQ(frame[frame.size() - 1], 0xCBU);
  EXPECT_EQ(rm_referee::crc16(frame.data(), frame.size() - 2), 0xCB66U);
}

TEST(Frame, BuildsProtocolReferenceFrame)
{
  // 0x0001 with 11 zero bytes and seq 1: the worked example from the protocol.
  EXPECT_EQ(to_hex(make_reference_frame()), "a50b00015c0100000000000000000000000066cb");
}

TEST(FrameParser, ParsesFrameFedOneByteAtATime)
{
  rm_referee::FrameParser parser;
  std::vector<rm_referee::Frame> frames;
  const auto callback = [&frames](rm_referee::Frame && frame) {
      frames.push_back(std::move(frame));
    };

  const std::vector<std::uint8_t> bytes = make_reference_frame();
  for (const std::uint8_t byte : bytes) {
    parser.feed(&byte, 1, callback);
  }

  ASSERT_EQ(frames.size(), 1U);
  EXPECT_EQ(frames[0].cmd_id, 0x0001);
  EXPECT_EQ(frames[0].seq, 1U);
  ASSERT_EQ(frames[0].payload.size(), 11U);
  EXPECT_EQ(parser.stats().frames, 1U);
  EXPECT_EQ(parser.stats().crc8_errors, 0U);
  EXPECT_EQ(parser.stats().crc16_errors, 0U);
}

TEST(FrameParser, ResynchronisesAfterGarbage)
{
  rm_referee::FrameParser parser;
  std::vector<rm_referee::Frame> frames;
  const auto callback = [&frames](rm_referee::Frame && frame) {
      frames.push_back(std::move(frame));
    };

  std::vector<std::uint8_t> stream = {0x11, 0x22, 0x33};
  const std::vector<std::uint8_t> frame = make_reference_frame();
  stream.insert(stream.end(), frame.begin(), frame.end());
  parser.feed(stream, callback);

  ASSERT_EQ(frames.size(), 1U);
  EXPECT_EQ(frames[0].cmd_id, 0x0001);
  EXPECT_EQ(parser.stats().dropped_bytes, 3U);
}

TEST(FrameParser, RejectsBadCrc8AndRecovers)
{
  rm_referee::FrameParser parser;
  std::vector<rm_referee::Frame> frames;
  const auto callback = [&frames](rm_referee::Frame && frame) {
      frames.push_back(std::move(frame));
    };

  std::vector<std::uint8_t> bad = make_reference_frame();
  bad[4] ^= 0xFFU;
  parser.feed(bad, callback);
  EXPECT_TRUE(frames.empty());
  EXPECT_EQ(parser.stats().crc8_errors, 1U);

  parser.feed(make_reference_frame(), callback);
  EXPECT_EQ(frames.size(), 1U);
}

TEST(FrameParser, RejectsBadCrc16AndRecovers)
{
  rm_referee::FrameParser parser;
  std::vector<rm_referee::Frame> frames;
  const auto callback = [&frames](rm_referee::Frame && frame) {
      frames.push_back(std::move(frame));
    };

  std::vector<std::uint8_t> bad = make_reference_frame();
  bad.back() ^= 0xFFU;
  parser.feed(bad, callback);
  EXPECT_TRUE(frames.empty());
  EXPECT_EQ(parser.stats().crc16_errors, 1U);

  parser.feed(make_reference_frame(), callback);
  EXPECT_EQ(frames.size(), 1U);
}

TEST(FrameParser, RejectsOversizedPayloadLength)
{
  rm_referee::FrameParser parser(118);
  std::vector<rm_referee::Frame> frames;
  const auto callback = [&frames](rm_referee::Frame && frame) {
      frames.push_back(std::move(frame));
    };

  std::vector<std::uint8_t> header = {0xA5, 0xFF, 0x00, 0x01};
  header.push_back(rm_referee::crc8(header.data(), header.size()));
  parser.feed(header, callback);
  EXPECT_TRUE(frames.empty());
  EXPECT_EQ(parser.stats().oversize_frames, 1U);

  parser.feed(make_reference_frame(), callback);
  EXPECT_EQ(frames.size(), 1U);
}

TEST(Codec, GameStatusRoundTrip)
{
  rm_referee::cmd::GameStatus status{};
  status.game_type_and_progress = 0x35;
  status.stage_remain_time = 180;
  status.sync_time_stamp = 0x0102030405060708ULL;

  const std::vector<std::uint8_t> bytes = rm_referee::codec::encode(status);
  ASSERT_EQ(bytes.size(), 11U);

  const auto decoded =
    rm_referee::codec::decode<rm_referee::cmd::GameStatus>(bytes.data(), bytes.size());
  ASSERT_TRUE(decoded.has_value());
  EXPECT_EQ(decoded->game_type(), 5U);
  EXPECT_EQ(decoded->game_progress(), 3U);
  EXPECT_EQ(decoded->stage_remain_time, 180U);
  EXPECT_EQ(decoded->sync_time_stamp, 0x0102030405060708ULL);

  EXPECT_FALSE(
    rm_referee::codec::decode<rm_referee::cmd::GameStatus>(bytes.data(), bytes.size() - 1));
}

TEST(Codec, RobotStatusDecodesPowerOutput)
{
  rm_referee::cmd::RobotStatus status{};
  status.robot_id = 7;
  status.current_hp = 200;
  status.maximum_hp = 400;
  status.bullet_speed_limit = 15.0F;
  status.power_output = 0x05;

  const std::vector<std::uint8_t> bytes = rm_referee::codec::encode(status);
  const auto decoded =
    rm_referee::codec::decode<rm_referee::cmd::RobotStatus>(bytes.data(), bytes.size());
  ASSERT_TRUE(decoded.has_value());
  EXPECT_EQ(decoded->robot_id, 7U);
  EXPECT_EQ(decoded->current_hp, 200U);
  EXPECT_FLOAT_EQ(decoded->bullet_speed_limit, 15.0F);
  EXPECT_TRUE(decoded->gimbal_output());
  EXPECT_FALSE(decoded->chassis_output());
  EXPECT_TRUE(decoded->shooter_output());
}

TEST(Codec, RobotInteractionIsVariableLength)
{
  rm_referee::cmd::RobotInteraction interaction;
  interaction.data_cmd_id = 0x0123;
  interaction.sender_id = 9;
  interaction.receiver_id = 0x8080;
  interaction.user_data = {1, 2, 3, 4, 5};

  const std::vector<std::uint8_t> bytes = rm_referee::codec::encode(interaction);
  ASSERT_EQ(bytes.size(), 11U);
  EXPECT_EQ(rm_referee::codec::encoded_size(interaction), 11U);

  const auto decoded =
    rm_referee::codec::decode<rm_referee::cmd::RobotInteraction>(bytes.data(), bytes.size());
  ASSERT_TRUE(decoded.has_value());
  EXPECT_EQ(decoded->data_cmd_id, 0x0123U);
  EXPECT_EQ(decoded->sender_id, 9U);
  EXPECT_EQ(decoded->receiver_id, 0x8080U);
  EXPECT_EQ(decoded->user_data, interaction.user_data);

  EXPECT_FALSE(rm_referee::codec::decode<rm_referee::cmd::RobotInteraction>(bytes.data(), 5));
}

TEST(Codec, RejectsOversizedRobotInteraction)
{
  rm_referee::cmd::RobotInteraction interaction;
  interaction.user_data.assign(rm_referee::cmd::RobotInteraction::kMaxUserData + 1, 0);
  EXPECT_TRUE(rm_referee::codec::encode(interaction).empty());
}

TEST(Commands, SentryInfoBitFields)
{
  rm_referee::cmd::SentryInfo info{};
  info.sentry_info = 300U | (1U << 19U) | (1U << 20U) | (5U << 21U);
  info.sentry_info_2 = 1U | (2U << 12U) | (1U << 14U);
  info.sentry_info_3 = 10U | (20U << 8U) | (30U << 16U) | (40ULL << 32U);

  EXPECT_EQ(info.exchanged_ammo(), 300U);
  EXPECT_TRUE(info.can_free_revive());
  EXPECT_TRUE(info.can_buy_revive());
  EXPECT_EQ(info.revive_cost(), 5U);
  EXPECT_TRUE(info.out_of_combat());
  EXPECT_EQ(info.posture(), 2U);
  EXPECT_TRUE(info.can_activate_energy());
  EXPECT_FALSE(info.is_enhanced());
  EXPECT_EQ(info.attack_weak_remaining(), 10U);
  EXPECT_EQ(info.defence_weak_remaining(), 20U);
  EXPECT_EQ(info.move_weak_remaining(), 30U);
  EXPECT_EQ(info.enhanced_attack_remaining(), 40U);
}

TEST(Commands, RobotHurtNibbles)
{
  rm_referee::cmd::RobotHurt hurt{};
  hurt.armor_id_and_reason = 0x23;
  EXPECT_EQ(hurt.armor_id(), 3U);
  EXPECT_EQ(hurt.deduction_reason(), 2U);
}
