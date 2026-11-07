#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iterator>

#include <geometry_msgs/msg/point.hpp>
#include <std_msgs/msg/header.hpp>

#include "rm_referee/commands.hpp"
#include "rm_referee_msgs/msg/dart_client_cmd.hpp"
#include "rm_referee_msgs/msg/dart_info.hpp"
#include "rm_referee_msgs/msg/field_event.hpp"
#include "rm_referee_msgs/msg/game_result.hpp"
#include "rm_referee_msgs/msg/game_robot_hp.hpp"
#include "rm_referee_msgs/msg/game_status.hpp"
#include "rm_referee_msgs/msg/ground_robot_position.hpp"
#include "rm_referee_msgs/msg/map_command.hpp"
#include "rm_referee_msgs/msg/map_data.hpp"
#include "rm_referee_msgs/msg/map_robot_data.hpp"
#include "rm_referee_msgs/msg/power_heat.hpp"
#include "rm_referee_msgs/msg/projectile_allowance.hpp"
#include "rm_referee_msgs/msg/radar_info.hpp"
#include "rm_referee_msgs/msg/radar_mark.hpp"
#include "rm_referee_msgs/msg/referee_warning.hpp"
#include "rm_referee_msgs/msg/rfid_status.hpp"
#include "rm_referee_msgs/msg/robot_buff.hpp"
#include "rm_referee_msgs/msg/robot_hurt.hpp"
#include "rm_referee_msgs/msg/robot_interaction.hpp"
#include "rm_referee_msgs/msg/robot_position.hpp"
#include "rm_referee_msgs/msg/robot_status.hpp"
#include "rm_referee_msgs/msg/sentry_info.hpp"
#include "rm_referee_msgs/msg/shoot.hpp"

namespace rm_referee::convert
{

inline geometry_msgs::msg::Point point(float x, float y)
{
  geometry_msgs::msg::Point p;
  p.x = x;
  p.y = y;
  p.z = 0.0F;
  return p;
}

inline rm_referee_msgs::msg::GameStatus to_msg(const cmd::GameStatus & in, const std_msgs::msg::Header & header)
{
  rm_referee_msgs::msg::GameStatus out;
  out.header = header;
  out.game_type = in.game_type();
  out.game_progress = in.game_progress();
  out.stage_remain_time = in.stage_remain_time;
  out.sync_time_stamp = in.sync_time_stamp;
  return out;
}

inline rm_referee_msgs::msg::GameResult to_msg(const cmd::GameResult & in, const std_msgs::msg::Header & header)
{
  rm_referee_msgs::msg::GameResult out;
  out.header = header;
  out.winner = in.winner;
  return out;
}

inline rm_referee_msgs::msg::GameRobotHP to_msg(
  const cmd::GameRobotHP & in, const std_msgs::msg::Header & header)
{
  rm_referee_msgs::msg::GameRobotHP out;
  out.header = header;
  out.ally_1_robot_hp = in.ally_1_robot_hp;
  out.ally_2_robot_hp = in.ally_2_robot_hp;
  out.ally_3_robot_hp = in.ally_3_robot_hp;
  out.ally_4_robot_hp = in.ally_4_robot_hp;
  out.damage_difference = in.damage_difference;
  out.ally_7_robot_hp = in.ally_7_robot_hp;
  out.outpost_r_hp = in.outpost_r_hp;
  out.ally_base_hp = in.ally_base_hp;
  out.outpost_b_hp = in.outpost_b_hp;
  out.enemy_base_hp = in.enemy_base_hp;
  out.ally_control_degree = in.ally_control_degree;
  out.enemy_control_degree = in.enemy_control_degree;
  return out;
}

inline rm_referee_msgs::msg::FieldEvent to_msg(const cmd::FieldEvent & in, const std_msgs::msg::Header & header)
{
  rm_referee_msgs::msg::FieldEvent out;
  out.header = header;
  out.event_data = in.event_data;
  out.event_data_2 = in.event_data_2;
  return out;
}

inline rm_referee_msgs::msg::RefereeWarning to_msg(
  const cmd::RefereeWarning & in, const std_msgs::msg::Header & header)
{
  rm_referee_msgs::msg::RefereeWarning out;
  out.header = header;
  out.level = in.level;
  out.offending_robot_id = in.offending_robot_id;
  out.count = in.count;
  return out;
}

inline rm_referee_msgs::msg::DartInfo to_msg(const cmd::DartInfo & in, const std_msgs::msg::Header & header)
{
  rm_referee_msgs::msg::DartInfo out;
  out.header = header;
  out.remaining_time = in.dart_remaining_time;
  out.recent_hit_target = in.recent_hit_target();
  out.enemy_hit_count = in.enemy_hit_count();
  out.selected_target = in.selected_target();
  return out;
}

inline rm_referee_msgs::msg::RobotStatus to_msg(
  const cmd::RobotStatus & in, const std_msgs::msg::Header & header)
{
  rm_referee_msgs::msg::RobotStatus out;
  out.header = header;
  out.robot_id = in.robot_id;
  out.robot_level = in.robot_level;
  out.current_hp = in.current_hp;
  out.maximum_hp = in.maximum_hp;
  out.shooter_barrel_cooling_value = in.shooter_barrel_cooling_value;
  out.shooter_barrel_heat_limit = in.shooter_barrel_heat_limit;
  out.chassis_power_limit = in.chassis_power_limit;
  out.bullet_speed_limit = in.bullet_speed_limit;
  out.gimbal_output = in.gimbal_output();
  out.chassis_output = in.chassis_output();
  out.shooter_output = in.shooter_output();
  return out;
}

inline rm_referee_msgs::msg::PowerHeat to_msg(const cmd::PowerHeat & in, const std_msgs::msg::Header & header)
{
  rm_referee_msgs::msg::PowerHeat out;
  out.header = header;
  out.buffer_energy = in.buffer_energy;
  out.shooter_17mm_barrel_heat = in.shooter_17mm_barrel_heat;
  out.shooter_42mm_barrel_heat = in.shooter_42mm_barrel_heat;
  return out;
}

inline rm_referee_msgs::msg::RobotPosition to_msg(
  const cmd::RobotPosition & in, const std_msgs::msg::Header & header)
{
  rm_referee_msgs::msg::RobotPosition out;
  out.header = header;
  out.x = in.x;
  out.y = in.y;
  out.angle = in.angle;
  return out;
}

inline rm_referee_msgs::msg::RobotBuff to_msg(const cmd::RobotBuff & in, const std_msgs::msg::Header & header)
{
  rm_referee_msgs::msg::RobotBuff out;
  out.header = header;
  out.recovery_buff = in.recovery_buff;
  out.cooling_buff = in.cooling_buff;
  out.defence_buff = in.defence_buff;
  out.vulnerability_buff = in.vulnerability_buff;
  out.attack_buff = in.attack_buff;
  return out;
}

inline rm_referee_msgs::msg::RobotHurt to_msg(const cmd::RobotHurt & in, const std_msgs::msg::Header & header)
{
  rm_referee_msgs::msg::RobotHurt out;
  out.header = header;
  out.armor_id = in.armor_id();
  out.deduction_reason = in.deduction_reason();
  out.hit_sequence = in.hit_sequence;
  out.hit_position_x = in.hit_position_x;
  out.hit_position_y = in.hit_position_y;
  return out;
}

inline rm_referee_msgs::msg::Shoot to_msg(const cmd::Shoot & in, const std_msgs::msg::Header & header)
{
  rm_referee_msgs::msg::Shoot out;
  out.header = header;
  out.bullet_type = in.bullet_type;
  out.shooter_number = in.shooter_number;
  out.launching_frequency = in.launching_frequency;
  out.initial_speed = in.initial_speed;
  return out;
}

inline rm_referee_msgs::msg::ProjectileAllowance to_msg(
  const cmd::ProjectileAllowance & in, const std_msgs::msg::Header & header)
{
  rm_referee_msgs::msg::ProjectileAllowance out;
  out.header = header;
  out.projectile_allowance_17mm = in.projectile_allowance_17mm;
  out.projectile_allowance_42mm = in.projectile_allowance_42mm;
  out.remaining_gold_coin = in.remaining_gold_coin;
  out.projectile_allowance_fortress = in.projectile_allowance_fortress;
  return out;
}

inline rm_referee_msgs::msg::RfidStatus to_msg(const cmd::RfidStatus & in, const std_msgs::msg::Header & header)
{
  rm_referee_msgs::msg::RfidStatus out;
  out.header = header;
  out.rfid_status = in.rfid_status;
  out.rfid_status_2 = in.rfid_status_2;
  return out;
}

inline rm_referee_msgs::msg::DartClientCmd to_msg(
  const cmd::DartClientCmd & in, const std_msgs::msg::Header & header)
{
  rm_referee_msgs::msg::DartClientCmd out;
  out.header = header;
  out.dart_launch_opening_status = in.dart_launch_opening_status;
  out.reserved = in.reserved;
  out.target_change_time = in.target_change_time;
  out.latest_launch_cmd_time = in.latest_launch_cmd_time;
  return out;
}

inline rm_referee_msgs::msg::SentryInfo to_msg(const cmd::SentryInfo & in, const std_msgs::msg::Header & header)
{
  rm_referee_msgs::msg::SentryInfo out;
  out.header = header;
  out.exchanged_ammo = in.exchanged_ammo();
  out.remote_exchange_ammo_count = in.remote_exchange_ammo_count();
  out.remote_exchange_hp_count = in.remote_exchange_hp_count();
  out.can_free_revive = in.can_free_revive();
  out.can_buy_revive = in.can_buy_revive();
  out.revive_cost = in.revive_cost();
  out.out_of_combat = in.out_of_combat();
  out.team_17mm_exchange_left = in.team_17mm_exchange_left();
  out.posture = in.posture();
  out.can_activate_energy = in.can_activate_energy();
  out.is_enhanced = in.is_enhanced();
  out.attack_weak_remaining = in.attack_weak_remaining();
  out.defence_weak_remaining = in.defence_weak_remaining();
  out.move_weak_remaining = in.move_weak_remaining();
  out.enhanced_attack_remaining = in.enhanced_attack_remaining();
  out.enhanced_defence_remaining = in.enhanced_defence_remaining();
  out.enhanced_move_remaining = in.enhanced_move_remaining();
  return out;
}

inline rm_referee_msgs::msg::RadarInfo to_msg(const cmd::RadarInfo & in, const std_msgs::msg::Header & header)
{
  rm_referee_msgs::msg::RadarInfo out;
  out.header = header;
  out.double_vulnerability_chances = in.double_vulnerability_chances();
  out.enemy_double_vulnerable = in.enemy_double_vulnerable();
  out.encryption_level = in.encryption_level();
  out.can_change_key = in.can_change_key();
  out.armor_fortification_active = in.armor_fortification_active();
  out.armor_fortification_chances = in.armor_fortification_chances();
  out.fortify_center_x = in.fortify_center_x;
  out.fortify_center_y = in.fortify_center_y;
  out.fortify_radius = in.fortify_radius;
  return out;
}

inline rm_referee_msgs::msg::RadarMark to_msg(const cmd::RadarMark & in, const std_msgs::msg::Header & header)
{
  rm_referee_msgs::msg::RadarMark out;
  out.header = header;
  out.mark_progress = in.mark_progress;
  return out;
}

inline rm_referee_msgs::msg::MapCommand to_msg(const cmd::MapCommand & in, const std_msgs::msg::Header & header)
{
  rm_referee_msgs::msg::MapCommand out;
  out.header = header;
  out.target_position_x = in.target_position_x;
  out.target_position_y = in.target_position_y;
  out.cmd_keyboard = in.cmd_keyboard;
  out.target_robot_id = in.target_robot_id;
  out.cmd_source = in.cmd_source;
  return out;
}

inline rm_referee_msgs::msg::GroundRobotPosition to_msg(
  const cmd::GroundRobotPosition & in, const std_msgs::msg::Header & header)
{
  rm_referee_msgs::msg::GroundRobotPosition out;
  out.header = header;
  out.ally_heavy = point(in.heavyengineer_x, in.heavyengineer_y);
  out.ally_standard_3 = point(in.standard_3_x, in.standard_3_y);
  out.ally_standard_4 = point(in.standard_4_x, in.standard_4_y);
  return out;
}

inline rm_referee_msgs::msg::MapRobotData to_msg(
  const cmd::MapRobotData & in, const std_msgs::msg::Header & header)
{
  rm_referee_msgs::msg::MapRobotData out;
  out.header = header;
  out.enemy_heavy = point(in.enemy_heavy_x, in.enemy_heavy_y);
  out.enemy_standard_3 = point(in.enemy_standard_3_x, in.enemy_standard_3_y);
  out.enemy_standard_4 = point(in.enemy_standard_4_x, in.enemy_standard_4_y);
  out.enemy_sentry = point(in.enemy_sentry_x, in.enemy_sentry_y);
  out.ally_heavy = point(in.ally_heavy_x, in.ally_heavy_y);
  out.ally_standard_3 = point(in.ally_standard_3_x, in.ally_standard_3_y);
  out.ally_standard_4 = point(in.ally_standard_4_x, in.ally_standard_4_y);
  out.ally_sentry = point(in.ally_sentry_x, in.ally_sentry_y);
  return out;
}

inline rm_referee_msgs::msg::MapData to_msg(const cmd::MapData & in, const std_msgs::msg::Header & header)
{
  rm_referee_msgs::msg::MapData out;
  out.header = header;
  out.intention = in.intention;
  out.start_position_x = in.start_position_x;
  out.start_position_y = in.start_position_y;
  std::copy(std::begin(in.delta_x), std::end(in.delta_x), out.delta_x.begin());
  std::copy(std::begin(in.delta_y), std::end(in.delta_y), out.delta_y.begin());
  out.sender_id = in.sender_id;
  return out;
}

inline rm_referee_msgs::msg::RobotInteraction to_msg(
  const cmd::RobotInteraction & in, const std_msgs::msg::Header & header)
{
  rm_referee_msgs::msg::RobotInteraction out;
  out.header = header;
  out.data_cmd_id = in.data_cmd_id;
  out.sender_id = in.sender_id;
  out.receiver_id = in.receiver_id;
  out.data = in.user_data;
  return out;
}

inline cmd::RobotInteraction from_msg(const rm_referee_msgs::msg::RobotInteraction & in)
{
  cmd::RobotInteraction out;
  out.data_cmd_id = in.data_cmd_id;
  out.sender_id = in.sender_id;
  out.receiver_id = in.receiver_id;
  out.user_data = in.data;
  return out;
}

inline std::uint16_t to_u16(float value)
{
  if (!std::isfinite(value) || value <= 0.0F) {
    return 0;
  }
  if (value >= 65535.0F) {
    return 65535;
  }
  return static_cast<std::uint16_t>(std::lround(value));
}

inline cmd::MapRobotData from_msg(const rm_referee_msgs::msg::MapRobotData & in)
{
  cmd::MapRobotData out{};
  out.enemy_heavy_x = to_u16(in.enemy_heavy.x);
  out.enemy_heavy_y = to_u16(in.enemy_heavy.y);
  out.enemy_standard_3_x = to_u16(in.enemy_standard_3.x);
  out.enemy_standard_3_y = to_u16(in.enemy_standard_3.y);
  out.enemy_standard_4_x = to_u16(in.enemy_standard_4.x);
  out.enemy_standard_4_y = to_u16(in.enemy_standard_4.y);
  out.enemy_sentry_x = to_u16(in.enemy_sentry.x);
  out.enemy_sentry_y = to_u16(in.enemy_sentry.y);
  out.ally_heavy_x = to_u16(in.ally_heavy.x);
  out.ally_heavy_y = to_u16(in.ally_heavy.y);
  out.ally_standard_3_x = to_u16(in.ally_standard_3.x);
  out.ally_standard_3_y = to_u16(in.ally_standard_3.y);
  out.ally_standard_4_x = to_u16(in.ally_standard_4.x);
  out.ally_standard_4_y = to_u16(in.ally_standard_4.y);
  out.ally_sentry_x = to_u16(in.ally_sentry.x);
  out.ally_sentry_y = to_u16(in.ally_sentry.y);
  return out;
}

}  // namespace rm_referee::convert
