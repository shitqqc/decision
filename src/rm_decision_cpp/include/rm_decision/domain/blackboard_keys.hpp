#pragma once

#include <cstdint>
#include <string>

namespace rm_decision
{
namespace BbKey
{
// Referee / online
inline constexpr const char * kHealth = "health";
inline constexpr const char * kGameProgress = "game_progress";
inline constexpr const char * kGameTimeRemaining = "game_time_remaining";
inline constexpr const char * kRemainingEnergy = "remaining_energy";
inline constexpr const char * kAllyOutpostHp = "ally_outpost_hp";
inline constexpr const char * kAllyBaseHp = "ally_base_hp";
inline constexpr const char * kEnemyOutpostHp = "enemy_outpost_hp";
inline constexpr const char * kEnemyBaseHp = "enemy_base_hp";
inline constexpr const char * kHurtHpDeductionReason = "hurt_hp_deduction_reason";
inline constexpr const char * kPostureFeedback = "posture_feedback";
inline constexpr const char * kOutOfCombat = "out_of_combat";
inline constexpr const char * kInCombat = "in_combat";
inline constexpr const char * kRedeemable17mm = "redeemable_17mm";
inline constexpr const char * kBulletsRemaining = "bullets_remaining";
inline constexpr const char * kEnergyActivatable = "energy_activatable";
inline constexpr const char * kCoinRemaining = "coin_remaining";
inline constexpr const char * kCurrentHeat = "current_heat";
inline constexpr const char * kHeatLimit = "heat_limit";
inline constexpr const char * kBigEnergyStatus = "big_energy_status";
inline constexpr const char * kControlMode = "control_mode";
inline constexpr const char * kManualGoalX = "manual_goal_x";
inline constexpr const char * kManualGoalY = "manual_goal_y";
inline constexpr const char * kCanFreeResurrect = "can_free_resurrect";
inline constexpr const char * kEnhancedAttackRemaining = "enhanced_attack_remaining_time";
inline constexpr const char * kEnhancedDefendRemaining = "enhanced_defend_remaining_time";
inline constexpr const char * kEnhancedMoveRemaining = "enhanced_move_remaining_time";

// Pose
// TODO: 这四个键目前是注释状态，OdomPoseIngress 也是整段注释（编译出空 TU，但仍占
// CMakeLists.txt 一行）。它们本身不是自瞄追击的必需项 —— 目标位置可直接来自
// /tracker/target 的 odom 系 position，本车位姿可用 tf2 查 map <- base_link。
// 但以下几种情况仍需要恢复本链路：
//   (a) 想避免引入 tf2_ros（当前包只依赖 tf2，没有 tf2_ros）；
//   (b) 需要「本车在区域内的判断」（之前删掉的 in_*_zone 那套）；
//   (c) 需要在 BT 节点里同步读到本车位姿，而不想每 tick 查 TF。
// 恢复顺序：
//   1. 取消下面四行注释；
//   2. 取消 src/infrastructure/odom_pose_ingress.cpp 与 .hpp 的整段注释；
//   3. 在 decision_app.cpp 里重新构造 OdomPoseIngress，并在 spinLoop() 里每 tick 调 update()；
//   4. initDefaults_ 里把 kPoseValid 置 false。
// 注意：/odom 与 map 之间可能有 map->odom 变换，直接取 odom 位姿写进这三个键会有漂移。
// inline constexpr const char * kCurrentPoseX = "current_pose_x";
// inline constexpr const char * kCurrentPoseY = "current_pose_y";
// inline constexpr const char * kCurrentPoseYaw = "current_pose_yaw";
// inline constexpr const char * kPoseValid = "pose_valid";
// Tunnel
inline constexpr const char * kThroughTunnel = "through_tunnel";

// Decision outputs / shared
inline constexpr const char * kTacticalMode = "tactical_mode";
inline constexpr const char * kNavMode = "nav_mode";
inline constexpr const char * kNavGoal = "nav_goal";
inline constexpr const char * kNavGoalValid = "nav_goal_valid";
inline constexpr const char * kDesiredStance = "desired_stance";
inline constexpr const char * kCurrentStance = "current_stance";
inline constexpr const char * kUseSpin = "use_spin";
inline constexpr const char * kUseSuperCap = "use_super_cap";
inline constexpr const char * kReviveRequest = "revive_request";
inline constexpr const char * kAmmoPurchaseTotal = "ammo_purchase_total";
inline constexpr const char * kBuyProjectileTimes = "buy_projectile_times";
inline constexpr const char * kBuyHpTimes = "buy_hp_times";
// inline constexpr const char * kInEnemyFortZone = "in_enemy_fort_zone";
// inline constexpr const char * kInOwnSupplyZone = "in_own_supply_zone";
// inline constexpr const char * kInOwnOutpostZone = "in_own_outpost_zone";
inline constexpr const char * kTargetValid = "target_valid";
inline constexpr const char * kTargetPoseX = "target_pose_x";
inline constexpr const char * kTargetPoseY = "target_pose_y";
// TODO: 自瞄入口。kTargetValid/kTargetPoseX/kTargetPoseY 目前只在 initDefaults_() 里置过，
// 没有任何 ingress 真正写入，所以 CheckTargetVisible 永远是 FAILURE。
// 数据源（已确认，见 /home/shitqqc/PinyVision）：
//   /tracker/target，类型 rm_interfaces/msg/Target，header.frame_id 为 odom。
//   其中 position 是 odom 系目标中心（m），velocity 是 odom 系速度，tracking 表示有无目标，
//   id 为 "one".."five"/"sentry"/"base"/"outpost"。该消息没有 distance 字段，距离需自算。
// 写入约定：
//   kTargetValid  <- Target.tracking
//   kTargetPoseX / kTargetPoseY <- Target.position 经 map<-odom 变换后的 map 系坐标
//   kTargetDistance <- 目标与本车在 map 系下的距离（用于追击/停车门限）
inline constexpr const char * kTargetYaw = "target_yaw";
inline constexpr const char * kTargetPitch = "target_pitch";
/// 目标到本车的距离（map 系），用于追击时的靠近/停车判定。
inline constexpr const char * kTargetDistance = "target_distance";

// bit-port extensions (blackboard-level; egress may not consume yet)
inline constexpr const char * kEnemyOutpostDestroyed = "enemy_outpost_destroyed";
inline constexpr const char * kHeroGuardActive = "hero_guard_active";
inline constexpr const char * kHighlandFallbackActive = "highland_fallback_active";
inline constexpr const char * kOutpostAutoAttackActive = "outpost_auto_attack_active";
inline constexpr const char * kOutpostManualAttackActive = "outpost_manual_attack_active";
inline constexpr const char * kOutpostRetreatActive = "outpost_retreat_active";
inline constexpr const char * kOutpostEnhancedDefendActive = "outpost_enhanced_defend_active";
inline constexpr const char * kTargetArmorId = "target_armor_id";
inline constexpr const char * kCapacitorCapacity = "capacitor_capacity";
inline constexpr const char * kManualStanceOverrideActive = "manual_stance_override_active";
inline constexpr const char * kManualStanceOverride = "manual_stance_override";
inline constexpr const char * kRemainingAmmoExchange = "remaining_ammo_exchange";
inline constexpr const char * kRemoteAmmoExchangeCount = "remote_ammo_exchange_count";
inline constexpr const char * kRemoteHealthExchangeCount = "remote_health_exchange_count";
// TODO: kGimbalYawCmd / kGimbalPitchCmd / kGimbalEnabled 目前只由 SetGimbalPose 写入，
// SentryCmdEgress 从不读取，所以云台指令只能靠 BT 节点自己发的 publisher 到达导航。
// 等云台输出并入 egress 时需注意：kGimbalEnabled 只是「启用/关闭云台控制」的开关，
// 关闭时必须显式写 false，避免子树失败后下游继续沿用上一帧的「启用」状态
// —— 单靠 yaw/pitch 是表达不了「关闭」的。
// 语义边界：这三个键只服务于「决策发固定观测角」。追击时云台归自瞄闭环，
// 决策既不下发角也不置 true，否则会和自瞄抢控制权。
inline constexpr const char * kGimbalYawCmd = "gimbal_yaw_cmd";
inline constexpr const char * kGimbalPitchCmd = "gimbal_pitch_cmd";
/// 云台控制开关：true = 接管云台并按 yaw/pitch 转，false = 交还控制权（yaw/pitch 不生效）。
inline constexpr const char * kGimbalEnabled = "gimbal_enabled";
inline constexpr const char * kEnergyAimMode = "energy_aim_mode";

} // namespace BbKey


enum class TacticalMode_e : std::uint8_t {
    Normal = 0,
    Attack = 1,
    Defend = 2,
};


enum class NavMode_e : std::uint8_t {
    Patrol = 0,
    Retreat = 1,
    Response = 2,
    Manual = 3,
};


enum class StanceCmd_e : std::uint8_t {
    Attack = 1,
    Defend = 2,
    Move = 3,
    EnhancedAttack = 4,
    EnhancedDefend = 5,
    EnhancedMove = 6,
};


enum class ControlMode_e : std::uint8_t {
    Auto = 0,
    Manual = 1,
};

} // namespace rm_decision
