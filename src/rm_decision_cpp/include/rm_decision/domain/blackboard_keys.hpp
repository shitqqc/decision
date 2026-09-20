#pragma once

#include <cstdint>
#include <string>

namespace rm_decision
{
namespace BbKey
{
// Referee / online
inline constexpr const char * kHealth = "health";
inline constexpr const char * kCurrentHp = "current_hp";
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
inline constexpr const char * kIsDisengaged = "is_disengaged";
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

// Pose
inline constexpr const char * kCurrentPoseX = "current_pose_x";
inline constexpr const char * kCurrentPoseY = "current_pose_y";
inline constexpr const char * kCurrentPoseYaw = "current_pose_yaw";
inline constexpr const char * kPoseValid = "pose_valid";

// Decision outputs / shared
inline constexpr const char * kTacticalMode = "tactical_mode";
inline constexpr const char * kNavMode = "current_mode";
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
inline constexpr const char * kInEnemyFortZone = "in_enemy_fort_zone";
inline constexpr const char * kInOwnSupplyZone = "in_own_supply_zone";
inline constexpr const char * kInOwnOutpostZone = "in_own_outpost_zone";

// bit-port extensions (blackboard-level; egress may not consume yet)
inline constexpr const char * kEnemyOutpostDestroyed = "enemy_outpost_destroyed";
inline constexpr const char * kOutpostAttackState = "outpost_attack_state";
inline constexpr const char * kHeroGuardActive = "hero_guard_active";
inline constexpr const char * kHighlandFallbackActive = "highland_fallback_active";
inline constexpr const char * kOutpostAutoAttackActive = "outpost_auto_attack_active";
inline constexpr const char * kOutpostManualAttackActive = "outpost_manual_attack_active";
inline constexpr const char * kOutpostRetreatActive = "outpost_retreat_active";
inline constexpr const char * kOutpostEnhancedDefendActive = "outpost_enhanced_defend_active";
inline constexpr const char * kPatrolIndex = "patrol_index";
inline constexpr const char * kPatrolBranch = "patrol_branch";
inline constexpr const char * kTargetValid = "target_valid";
inline constexpr const char * kTargetArmorId = "target_armor_id";
inline constexpr const char * kTargetPoseX = "target_pose_x";
inline constexpr const char * kTargetPoseY = "target_pose_y";
inline constexpr const char * kNotAimEnemy = "not_aim_enemy";
inline constexpr const char * kCapacitorCapacity = "capacitor_capacity";
inline constexpr const char * kManualStanceOverrideActive = "manual_stance_override_active";
inline constexpr const char * kManualStanceOverride = "manual_stance_override";
inline constexpr const char * kEnhancedAttackRemaining = "enhanced_attack_remaining_time";
inline constexpr const char * kEnhancedDefendRemaining = "enhanced_defend_remaining_time";
inline constexpr const char * kEnhancedMoveRemaining = "enhanced_move_remaining_time";
inline constexpr const char * kRemainingAmmoExchange = "remaining_ammo_exchange";
inline constexpr const char * kRemoteAmmoExchangeCount = "remote_ammo_exchange_count";
inline constexpr const char * kRemoteHealthExchangeCount = "remote_health_exchange_count";
inline constexpr const char * kThroughTunnel = "through_tunnel";
inline constexpr const char * kCurrentInTunnel = "current_in_tunnel";
inline constexpr const char * kTunnelEscapeActive = "tunnel_escape_active";
inline constexpr const char * kTunnelAlignActive = "tunnel_align_active";
inline constexpr const char * kGimbalYawCmd = "gimbal_yaw_cmd";
inline constexpr const char * kGimbalPitchCmd = "gimbal_pitch_cmd";
inline constexpr const char * kPitchMode = "pitch_mode";
inline constexpr const char * kScanYawMin = "scan_yaw_min";
inline constexpr const char * kScanYawMax = "scan_yaw_max";

}  // namespace BbKey


enum class TacticalMode_e : std::uint8_t
{
  Normal = 0,
  Attack = 1,
  Defend = 2,
};


enum class NavMode_e : std::uint8_t
{
  Patrol = 0,
  Retreat = 1,
  Response = 2,
  Manual = 3,
};


enum class StanceCmd_e : std::uint8_t
{
  Attack = 1,
  Defend = 2,
  Move = 3,
  EnhancedAttack = 4,
  EnhancedDefend = 5,
  EnhancedMove = 6,
};


enum class NavGoalId_e : int
{
  Home = 0,
  Bonus = 1,
  EnemyOutpost = 2,
  OwnFort = 3,
  EnemyFort = 4,
  OwnOutpost = 5,
  HeroGuard = 6,
};


enum class ControlMode_e : std::uint8_t
{
  Auto = 0,
  Manual = 1,
};

}  // namespace rm_decision
