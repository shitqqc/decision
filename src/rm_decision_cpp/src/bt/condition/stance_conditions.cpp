#include "rm_decision/bt/condition/stance_conditions.hpp"

#include "rm_decision/domain/blackboard_keys.hpp"

#include <cstdint>

namespace rm_decision
{
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
