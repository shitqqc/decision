#include "rm_decision/bt/condition/combat_conditions.hpp"

#include "rm_decision/domain/blackboard_keys.hpp"

#include <cmath>
#include <cstdint>
#include <geometry_msgs/msg/pose_stamped.hpp>

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
    bool in_combat = false;
    (void)config().blackboard->get(BbKey::kInCombat, in_combat);
    return (in_combat == expected) ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
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


} // namespace rm_decision
