#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

/// Wire-format payloads of the RoboMaster 2027 standard-link commands.
///
/// All multi-byte values are little-endian (the referee system and the
/// RoboMaster control boards are little-endian), and every struct is packed
/// exactly like the protocol definition. `static_assert`s pin the sizes.
namespace rm_referee::cmd
{

#pragma pack(push, 1)

  /// 0x0001 game_status. Low nibble: game type, high nibble: game progress.
  struct GameStatus
  {
    static constexpr std::uint16_t kId = 0x0001;

    std::uint8_t game_type_and_progress;
    std::uint16_t stage_remain_time;
    std::uint64_t sync_time_stamp;

    std::uint8_t game_type() const { return game_type_and_progress & 0x0FU; }
    std::uint8_t game_progress() const { return (game_type_and_progress >> 4U) & 0x0FU; }
  };
  static_assert(sizeof(GameStatus) == 11);

  /// 0x0002 game_result.
  struct GameResult
  {
    static constexpr std::uint16_t kId = 0x0002;

    std::uint8_t winner;
  };
  static_assert(sizeof(GameResult) == 1);

  /// 0x0003 game_robot_HP. ally_2 is reserved in the 2027 protocol.
  struct GameRobotHP
  {
    static constexpr std::uint16_t kId = 0x0003;

    std::uint16_t ally_1_robot_hp;
    std::uint16_t ally_2_robot_hp;
    std::uint16_t ally_3_robot_hp;
    std::uint16_t ally_4_robot_hp;
    std::int16_t damage_difference;
    std::uint16_t ally_7_robot_hp;
    std::uint16_t outpost_r_hp;
    std::uint16_t ally_base_hp;
    std::uint16_t outpost_b_hp;
    std::uint16_t enemy_base_hp;
    std::uint16_t ally_control_degree;
    std::uint16_t enemy_control_degree;
  };
  static_assert(sizeof(GameRobotHP) == 24);

  /// 0x0101 event_data.
  struct FieldEvent
  {
    static constexpr std::uint16_t kId = 0x0101;

    std::uint32_t event_data;
    std::uint32_t event_data_2;
  };
  static_assert(sizeof(FieldEvent) == 8);

  /// 0x0104 referee_warning.
  struct RefereeWarning
  {
    static constexpr std::uint16_t kId = 0x0104;

    std::uint8_t level;
    std::uint8_t offending_robot_id;
    std::uint8_t count;
  };
  static_assert(sizeof(RefereeWarning) == 3);

  /// 0x0105 dart_info.
  struct DartInfo
  {
    static constexpr std::uint16_t kId = 0x0105;

    std::uint8_t dart_remaining_time;
    std::uint16_t dart_info;

    std::uint8_t recent_hit_target() const { return static_cast<std::uint8_t>(dart_info & 0x07U); }
    std::uint8_t enemy_hit_count() const
    {
      return static_cast<std::uint8_t>((dart_info >> 3U) & 0x07U);
    }
    std::uint8_t selected_target() const
    {
      return static_cast<std::uint8_t>((dart_info >> 6U) & 0x07U);
    }
  };
  static_assert(sizeof(DartInfo) == 3);

  /// 0x0201 robot_status.
  struct RobotStatus
  {
    static constexpr std::uint16_t kId = 0x0201;

    std::uint8_t robot_id;
    std::uint8_t robot_level;
    std::uint16_t current_hp;
    std::uint16_t maximum_hp;
    std::uint16_t shooter_barrel_cooling_value;
    std::uint16_t shooter_barrel_heat_limit;
    std::uint16_t chassis_power_limit;
    float bullet_speed_limit;
    std::uint8_t power_output;

    bool gimbal_output() const { return (power_output & 0x01U) != 0; }
    bool chassis_output() const { return (power_output & 0x02U) != 0; }
    bool shooter_output() const { return (power_output & 0x04U) != 0; }
  };
  static_assert(sizeof(RobotStatus) == 17);

  /// 0x0202 power_heat. The first 8 bytes are reserved in the 2027 protocol.
  struct PowerHeat
  {
    static constexpr std::uint16_t kId = 0x0202;

    std::uint16_t reserved0;
    std::uint16_t reserved1;
    float reserved2;
    std::uint16_t buffer_energy;
    std::uint16_t shooter_17mm_barrel_heat;
    std::uint16_t shooter_42mm_barrel_heat;
  };
  static_assert(sizeof(PowerHeat) == 14);

  /// 0x0203 robot_pos. Units: metres and degrees (0 = north).
  struct RobotPosition
  {
    static constexpr std::uint16_t kId = 0x0203;

    float x;
    float y;
    float angle;
  };
  static_assert(sizeof(RobotPosition) == 12);

  /// 0x0204 buff.
  struct RobotBuff
  {
    static constexpr std::uint16_t kId = 0x0204;

    std::uint8_t recovery_buff;
    std::uint16_t cooling_buff;
    std::uint8_t defence_buff;
    std::uint8_t vulnerability_buff;
    std::uint16_t attack_buff;
  };
  static_assert(sizeof(RobotBuff) == 7);

  /// 0x0206 hurt. Low nibble: armor id, high nibble: deduction reason.
  struct RobotHurt
  {
    static constexpr std::uint16_t kId = 0x0206;

    std::uint8_t armor_id_and_reason;
    std::uint16_t hit_sequence;
    std::int16_t hit_position_x;
    std::int16_t hit_position_y;

    std::uint8_t armor_id() const { return armor_id_and_reason & 0x0FU; }
    std::uint8_t deduction_reason() const
    {
      return static_cast<std::uint8_t>((armor_id_and_reason >> 4U) & 0x0FU);
    }
  };
  static_assert(sizeof(RobotHurt) == 7);

  /// 0x0207 shoot.
  struct Shoot
  {
    static constexpr std::uint16_t kId = 0x0207;

    std::uint8_t bullet_type;
    std::uint8_t shooter_number;
    std::uint8_t launching_frequency;
    float initial_speed;
  };
  static_assert(sizeof(Shoot) == 7);

  /// 0x0208 projectile_allowance.
  struct ProjectileAllowance
  {
    static constexpr std::uint16_t kId = 0x0208;

    std::uint16_t projectile_allowance_17mm;
    std::uint16_t projectile_allowance_42mm;
    std::uint16_t remaining_gold_coin;
    std::uint16_t projectile_allowance_fortress;
  };
  static_assert(sizeof(ProjectileAllowance) == 8);

  /// 0x0209 rfid_status.
  struct RfidStatus
  {
    static constexpr std::uint16_t kId = 0x0209;

    std::uint32_t rfid_status;
    std::uint16_t rfid_status_2;
  };
  static_assert(sizeof(RfidStatus) == 6);

  /// 0x020A dart_client_cmd.
  struct DartClientCmd
  {
    static constexpr std::uint16_t kId = 0x020A;

    std::uint8_t dart_launch_opening_status;
    std::uint8_t reserved;
    std::uint16_t target_change_time;
    std::uint16_t latest_launch_cmd_time;
  };
  static_assert(sizeof(DartClientCmd) == 6);

  /// 0x020B ground_robot_position. Units: metres. Sent to the ally sentry.
  struct GroundRobotPosition
  {
    static constexpr std::uint16_t kId = 0x020B;

    float heavyengineer_x;
    float heavyengineer_y;
    float reserved0;
    float reserved1;
    float standard_3_x;
    float standard_3_y;
    float standard_4_x;
    float standard_4_y;
    float reserved2;
    float reserved3;
  };
  static_assert(sizeof(GroundRobotPosition) == 40);

  /// 0x020C radar_mark.
  struct RadarMark
  {
    static constexpr std::uint16_t kId = 0x020C;

    std::uint16_t mark_progress;
  };
  static_assert(sizeof(RadarMark) == 2);

  /// 0x020D sentry_info, exposed as the three raw words plus accessors.
  struct SentryInfo
  {
    static constexpr std::uint16_t kId = 0x020D;

    std::uint32_t sentry_info;
    std::uint16_t sentry_info_2;
    std::uint64_t sentry_info_3;

    std::uint16_t exchanged_ammo() const
    {
      return static_cast<std::uint16_t>(sentry_info & 0x07FFU);
    }
    std::uint8_t remote_exchange_ammo_count() const
    {
      return static_cast<std::uint8_t>((sentry_info >> 11U) & 0x0FU);
    }
    std::uint8_t remote_exchange_hp_count() const
    {
      return static_cast<std::uint8_t>((sentry_info >> 15U) & 0x0FU);
    }
    bool can_free_revive() const { return ((sentry_info >> 19U) & 0x01U) != 0; }
    bool can_buy_revive() const { return ((sentry_info >> 20U) & 0x01U) != 0; }
    std::uint16_t revive_cost() const
    {
      return static_cast<std::uint16_t>((sentry_info >> 21U) & 0x03FFU);
    }
    bool out_of_combat() const { return (sentry_info_2 & 0x0001U) != 0; }
    std::uint16_t team_17mm_exchange_left() const
    {
      return static_cast<std::uint16_t>((sentry_info_2 >> 1U) & 0x07FFU);
    }
    std::uint8_t posture() const
    {
      return static_cast<std::uint8_t>((sentry_info_2 >> 12U) & 0x03U);
    }
    bool can_activate_energy() const { return ((sentry_info_2 >> 14U) & 0x01U) != 0; }
    bool is_enhanced() const { return ((sentry_info_2 >> 15U) & 0x01U) != 0; }
    std::uint8_t attack_weak_remaining() const
    {
      return static_cast<std::uint8_t>(sentry_info_3 & 0xFFU);
    }
    std::uint8_t defence_weak_remaining() const
    {
      return static_cast<std::uint8_t>((sentry_info_3 >> 8U) & 0xFFU);
    }
    std::uint8_t move_weak_remaining() const
    {
      return static_cast<std::uint8_t>((sentry_info_3 >> 16U) & 0xFFU);
    }
    std::uint8_t enhanced_attack_remaining() const
    {
      return static_cast<std::uint8_t>((sentry_info_3 >> 32U) & 0xFFU);
    }
    std::uint8_t enhanced_defence_remaining() const
    {
      return static_cast<std::uint8_t>((sentry_info_3 >> 40U) & 0xFFU);
    }
    std::uint8_t enhanced_move_remaining() const
    {
      return static_cast<std::uint8_t>((sentry_info_3 >> 48U) & 0xFFU);
    }
  };
  static_assert(sizeof(SentryInfo) == 14);

  /// 0x020E radar_info.
  struct RadarInfo
  {
    static constexpr std::uint16_t kId = 0x020E;

    std::uint8_t radar_info;
    std::uint8_t radar_info_2;
    std::uint16_t fortify_center_x;
    std::uint16_t fortify_center_y;
    std::uint16_t fortify_radius;

    std::uint8_t double_vulnerability_chances() const { return radar_info & 0x03U; }
    bool enemy_double_vulnerable() const { return ((radar_info >> 2U) & 0x01U) != 0; }
    std::uint8_t encryption_level() const
    {
      return static_cast<std::uint8_t>((radar_info >> 3U) & 0x03U);
    }
    bool can_change_key() const { return ((radar_info >> 5U) & 0x01U) != 0; }
    bool armor_fortification_active() const { return (radar_info_2 & 0x01U) != 0; }
    std::uint8_t armor_fortification_chances() const
    {
      return static_cast<std::uint8_t>((radar_info_2 >> 1U) & 0x07U);
    }
  };
  static_assert(sizeof(RadarInfo) == 8);

  /// 0x0301 robot_interaction. Variable length: 6-byte header + up to 112 B data.
  struct RobotInteraction
  {
    static constexpr std::uint16_t kId = 0x0301;
    static constexpr std::size_t kHeaderLen = 6;
    static constexpr std::size_t kMaxUserData = 112;

    std::uint16_t data_cmd_id{0};
    std::uint16_t sender_id{0};
    std::uint16_t receiver_id{0};
    std::vector<std::uint8_t> user_data;
  };

  /// 0x0303 map_command.
  struct MapCommand
  {
    static constexpr std::uint16_t kId = 0x0303;

    float target_position_x;
    float target_position_y;
    std::uint8_t cmd_keyboard;
    std::uint8_t target_robot_id;
    std::uint16_t cmd_source;
  };
  static_assert(sizeof(MapCommand) == 12);

  /// 0x0305 map_robot_data. Units: centimetres. Reserved slots keep the wire layout.
  struct MapRobotData
  {
    static constexpr std::uint16_t kId = 0x0305;

    std::uint16_t enemy_heavy_x;
    std::uint16_t enemy_heavy_y;
    std::uint16_t reserved0;
    std::uint16_t reserved1;
    std::uint16_t enemy_standard_3_x;
    std::uint16_t enemy_standard_3_y;
    std::uint16_t enemy_standard_4_x;
    std::uint16_t enemy_standard_4_y;
    std::uint16_t reserved2;
    std::uint16_t reserved3;
    std::uint16_t enemy_sentry_x;
    std::uint16_t enemy_sentry_y;
    std::uint16_t ally_heavy_x;
    std::uint16_t ally_heavy_y;
    std::uint16_t reserved4;
    std::uint16_t reserved5;
    std::uint16_t ally_standard_3_x;
    std::uint16_t ally_standard_3_y;
    std::uint16_t ally_standard_4_x;
    std::uint16_t ally_standard_4_y;
    std::uint16_t reserved6;
    std::uint16_t reserved7;
    std::uint16_t ally_sentry_x;
    std::uint16_t ally_sentry_y;
  };
  static_assert(sizeof(MapRobotData) == 48);

  /// 0x0307 map_data. Units: decimetres.
  struct MapData
  {
    static constexpr std::uint16_t kId = 0x0307;
    static constexpr std::size_t kPathPoints = 49;

    std::uint8_t intention;
    std::uint16_t start_position_x;
    std::uint16_t start_position_y;
    std::int8_t delta_x[kPathPoints];
    std::int8_t delta_y[kPathPoints];
    std::uint16_t sender_id;
  };
  static_assert(sizeof(MapData) == 105);

  /// 0x0308 custom_info. user_data is UTF-16.
  struct CustomInfo
  {
    static constexpr std::uint16_t kId = 0x0308;

    std::uint16_t sender_id;
    std::uint16_t receiver_id;
    std::uint8_t user_data[30];
  };
  static_assert(sizeof(CustomInfo) == 34);

#pragma pack(pop)

} // namespace rm_referee::cmd
