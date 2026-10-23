#include "rm_decision/bt/condition/change_stance_condition.hpp"
#include "rm_decision/domain/blackboard_keys.hpp"
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <cmath>
#include <cstdint>
namespace rm_decision
{

CheckHeat::CheckHeat(const std::string & n, const BT::NodeConfig & c) : ConditionNode(n, c) {}
BT::PortsList CheckHeat::providedPorts()
{
    return {
        BT::InputPort<int>("threshold", 200, ""), BT::InputPort<std::string>("mode", "greater", "")};
}
BT::NodeStatus CheckHeat::tick()
{
    int thr = 200;
    std::string mode = "greater";
    (void)getInput("threshold", thr);
    (void)getInput("mode", mode);
    int heat = 0;
    (void)config().blackboard->get(BbKey::kCurrentHeat, heat);
    const bool ok = (mode == "less")         ? (heat < thr)
                    : (mode == "less_equal") ? (heat <= thr)
                                             : (heat > thr);
    return ok ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

CheckEngagedStatus::CheckEngagedStatus(const std::string & n, const BT::NodeConfig & c)
    : ConditionNode(n, c)
{
}
BT::PortsList CheckEngagedStatus::providedPorts()
{
    return {BT::InputPort<bool>("expected", true, "")};
}
BT::NodeStatus CheckEngagedStatus::tick()
{
    bool expected = true;
    (void)getInput("expected", expected);
    bool disengaged = false;
    (void)config().blackboard->get(BbKey::kIsDisengaged, disengaged);
    return ((!disengaged) == expected) ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}


CheckOutpostTarget::CheckOutpostTarget(const std::string & n, const BT::NodeConfig & c)
    : ConditionNode(n, c)
{
}
BT::PortsList CheckOutpostTarget::providedPorts()
{
    return {
        BT::InputPort<double>("goal_distance_threshold", 0.5, ""),
        BT::InputPort<bool>("require_pitch_up", false, ""),
    };
}
BT::NodeStatus CheckOutpostTarget::tick()
{
    (void)getInput<double>("goal_distance_threshold");
    (void)getInput<bool>("require_pitch_up");

    int armor = -1;
    bool valid = false;
    (void)config().blackboard->get(BbKey::kTargetArmorId, armor);
    (void)config().blackboard->get(BbKey::kTargetValid, valid);
    return (valid && armor == 1) ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}


CheckOutpostLowHealthDefend::CheckOutpostLowHealthDefend(const std::string & n, const BT::NodeConfig & c)
    : ConditionNode(n, c)
{
}
BT::PortsList CheckOutpostLowHealthDefend::providedPorts()
{
    return {
        BT::InputPort<float>("ally_outpost_hp", 200.f, ""),
        BT::InputPort<float>("health_drop_threshold", 50.f, ""),
        BT::InputPort<double>("goal_distance_threshold", 0.8, ""),
        BT::InputPort<double>("hold_seconds", 5.0, ""),
    };
}
BT::NodeStatus CheckOutpostLowHealthDefend::tick()
{
    float thr = 200.f;
    (void)getInput("ally_outpost_hp", thr);
    float hp = 0;
    (void)config().blackboard->get(BbKey::kAllyOutpostHp, hp);
    return hp > 0.f && hp < thr ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

CheckEnemyAreaRecentlyHurt::CheckEnemyAreaRecentlyHurt(const std::string & n, const BT::NodeConfig & c)
    : ConditionNode(n, c)
{
}
BT::PortsList CheckEnemyAreaRecentlyHurt::providedPorts()
{
    return {BT::InputPort<int>("reason", 0, ""), BT::InputPort<double>("hold_seconds", 0.0, "")};
}
BT::NodeStatus CheckEnemyAreaRecentlyHurt::tick()
{
    int reason = 0;
    (void)getInput("reason", reason);
    int got = 0;
    (void)config().blackboard->get(BbKey::kHurtHpDeductionReason, got);
    return (reason == 0 ? got != 0 : got == reason) ? BT::NodeStatus::SUCCESS
                                                    : BT::NodeStatus::FAILURE;
}

CheckTargetDistance::CheckTargetDistance(const std::string & n, const BT::NodeConfig & c)
    : ConditionNode(n, c)
{
}
BT::PortsList CheckTargetDistance::providedPorts()
{
    return {
        BT::InputPort<double>("max_distance", 5.0, ""),
        BT::InputPort<std::string>("mode", "less", "")};
}
BT::NodeStatus CheckTargetDistance::tick()
{
    double max_d = 5.0;
    std::string mode = "less";
    (void)getInput("max_distance", max_d);
    (void)getInput("mode", mode);
    bool tv = false, pv = false;
    double tx = 0, ty = 0, x = 0, y = 0;
    (void)config().blackboard->get(BbKey::kTargetValid, tv);
    (void)config().blackboard->get(BbKey::kPoseValid, pv);
    if (!tv || !pv)
        return BT::NodeStatus::FAILURE;
    (void)config().blackboard->get(BbKey::kTargetPoseX, tx);
    (void)config().blackboard->get(BbKey::kTargetPoseY, ty);
    (void)config().blackboard->get(BbKey::kCurrentPoseX, x);
    (void)config().blackboard->get(BbKey::kCurrentPoseY, y);
    const double d = std::hypot(tx - x, ty - y);
    return (mode == "greater" ? d > max_d : d < max_d) ? BT::NodeStatus::SUCCESS
                                                       : BT::NodeStatus::FAILURE;
}

CheckCrossZoneTransition::CheckCrossZoneTransition(
    const std::string & n, const BT::NodeConfig & c, const ZoneMap * z)
    : ConditionNode(n, c), zones_(z)
{
}
BT::NodeStatus CheckCrossZoneTransition::tick()
{
    double x = 0, y = 0;
    geometry_msgs::msg::PoseStamped nav_goal;
    bool through_tunnel = false;
    (void)config().blackboard->get(BbKey::kCurrentPoseX, x);
    (void)config().blackboard->get(BbKey::kCurrentPoseY, y);
    (void)config().blackboard->get(BbKey::kNavGoal, nav_goal);
    (void)config().blackboard->get(BbKey::kThroughTunnel, through_tunnel);

    const Point2d_s pt{x, y, 0.0};
    const Point2d_s goal_pt{nav_goal.pose.position.x, nav_goal.pose.position.y, 0.0};
    const bool in_highland =
        zones_ && zones_->inZone("highland", pt.x, pt.y);
    const bool in_own =
        zones_ && zones_->inZone("own_defense", pt.x, pt.y);
    const bool in_enemy =
        zones_ && zones_->inZone("enemy_defense", pt.x, pt.y);
    const bool goal_in_highland =
        zones_ && zones_->inZone("highland", goal_pt.x, goal_pt.y);
    const bool goal_in_own =
        zones_ && zones_->inZone("own_defense", goal_pt.x, goal_pt.y);
    const bool goal_in_enemy =
        zones_ && zones_->inZone("enemy_defense", goal_pt.x, goal_pt.y);
    const bool in_named = in_highland || in_own || in_enemy;
    const bool goal_in_named = goal_in_highland || goal_in_own || goal_in_enemy;
    const bool same_zone = (in_highland && goal_in_highland) ||
                           (in_own && goal_in_own) ||
                           (in_enemy && goal_in_enemy) ||
                           (!in_named && !goal_in_named);
    const bool need_cross_zone = !same_zone;
    const bool in_transform_zone = need_cross_zone || through_tunnel;
    return in_transform_zone ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

CheckCapacitorCapacity::CheckCapacitorCapacity(const std::string & n, const BT::NodeConfig & c)
    : ConditionNode(n, c)
{
}
BT::PortsList CheckCapacitorCapacity::providedPorts()
{
    return {
        BT::InputPort<float>("min_ratio", 0.3f, ""),
        BT::InputPort<float>("threshold", 0.3f, ""),
        BT::InputPort<std::string>("mode", "greater", ""),
    };
}
BT::NodeStatus CheckCapacitorCapacity::tick()
{
    float thr = 0.3f;
    std::string mode = "greater";
    if (!getInput("threshold", thr))
        (void)getInput("min_ratio", thr);
    (void)getInput("mode", mode);
    float cap = 1.f;
    (void)config().blackboard->get(BbKey::kCapacitorCapacity, cap);
    const bool ok = (mode == "less") ? (cap < thr) : (cap >= thr);
    return ok ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

CheckStanceEffectLimit::CheckStanceEffectLimit(const std::string & n, const BT::NodeConfig & c)
    : ConditionNode(n, c)
{
}
BT::PortsList CheckStanceEffectLimit::providedPorts()
{
    return {
        BT::InputPort<std::string>("stance", "ENHANCED_ATTACK", ""),
        BT::InputPort<std::string>("target_stance", "ENHANCED_ATTACK", ""),
        BT::InputPort<double>("max_time_sec", 0.0, ""),
    };
}
BT::NodeStatus CheckStanceEffectLimit::tick()
{
    std::string s = "ENHANCED_ATTACK";
    (void)getInput("stance", s);
    float rem = 0.f;
    if (s.find("ATTACK") != std::string::npos)
        (void)config().blackboard->get(BbKey::kEnhancedAttackRemaining, rem);
    else if (s.find("DEFEND") != std::string::npos)
        (void)config().blackboard->get(BbKey::kEnhancedDefendRemaining, rem);
    else
        (void)config().blackboard->get(BbKey::kEnhancedMoveRemaining, rem);
    return rem > 0.f ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

CheckStanceRefreshRequired::CheckStanceRefreshRequired(const std::string & n, const BT::NodeConfig & c)
    : ConditionNode(n, c)
{
}
BT::PortsList CheckStanceRefreshRequired::providedPorts()
{
    return {BT::OutputPort<std::string>("target_stance")};
}
BT::NodeStatus CheckStanceRefreshRequired::tick()
{
    std::uint8_t desired = 3, current = 3;
    (void)config().blackboard->get(BbKey::kDesiredStance, desired);
    (void)config().blackboard->get(BbKey::kCurrentStance, current);
    if (desired == current)
        return BT::NodeStatus::FAILURE;
    std::string name = "MOVE";
    if (desired == 1)
        name = "ATTACK";
    else if (desired == 2)
        name = "DEFEND";
    else if (desired == 4)
        name = "ENHANCED_ATTACK";
    else if (desired == 5)
        name = "ENHANCED_DEFEND";
    else if (desired == 6)
        name = "ENHANCED_MOVE";
    setOutput("target_stance", name);
    return BT::NodeStatus::SUCCESS;
}

CheckInEnemyFortZone::CheckInEnemyFortZone(const std::string & n, const BT::NodeConfig & c)
    : ConditionNode(n, c)
{
}
BT::NodeStatus CheckInEnemyFortZone::tick()
{
    bool v = false;
    (void)config().blackboard->get(BbKey::kInEnemyFortZone, v);
    return v ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

CheckEnemyDefenseHealthDrop::CheckEnemyDefenseHealthDrop(const std::string & n, const BT::NodeConfig & c)
    : ConditionNode(n, c)
{
}
BT::PortsList CheckEnemyDefenseHealthDrop::providedPorts()
{
    return {BT::InputPort<float>("drop", 50.f, ""), BT::InputPort<double>("stable_seconds", 5.0, "")};
}
BT::NodeStatus CheckEnemyDefenseHealthDrop::tick()
{
    // Approximate using hurt reason present
    int reason = 0;
    (void)config().blackboard->get(BbKey::kHurtHpDeductionReason, reason);
    return reason != 0 ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

CheckManualStanceOverride::CheckManualStanceOverride(const std::string & n, const BT::NodeConfig & c)
    : ConditionNode(n, c)
{
}
BT::NodeStatus CheckManualStanceOverride::tick()
{
    bool v = false;
    (void)config().blackboard->get(BbKey::kManualStanceOverrideActive, v);
    return v ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

CheckShouldEnhanceStance::CheckShouldEnhanceStance(const std::string & n, const BT::NodeConfig & c)
    : ConditionNode(n, c)
{
}
BT::PortsList CheckShouldEnhanceStance::providedPorts()
{
    return {
        BT::InputPort<std::string>("stance", "ENHANCED_ATTACK", ""),
        BT::InputPort<std::string>("target_stance", "DEFEND", ""),
        BT::InputPort<double>("min_remaining_sec", 1.0, ""),
        BT::InputPort<bool>("force_if_available", false, ""),
        BT::InputPort<double>("accumulated_threshold", 0.0, ""),
        BT::InputPort<int>("fallback_game_time", 0, ""),
    };
}
BT::NodeStatus CheckShouldEnhanceStance::tick()
{
    std::string s = "ENHANCED_ATTACK";
    (void)getInput("stance", s);
    float rem = 0.f;
    if (s.find("ATTACK") != std::string::npos)
        (void)config().blackboard->get(BbKey::kEnhancedAttackRemaining, rem);
    else if (s.find("DEFEND") != std::string::npos)
        (void)config().blackboard->get(BbKey::kEnhancedDefendRemaining, rem);
    else
        (void)config().blackboard->get(BbKey::kEnhancedMoveRemaining, rem);
    return rem > 1.f ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

} // namespace rm_decision
