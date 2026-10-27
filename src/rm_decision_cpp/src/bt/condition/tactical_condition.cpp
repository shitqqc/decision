#include "rm_decision/bt/condition/tactical_condition.hpp"
#include "rm_decision/domain/blackboard_keys.hpp"
#include <cstdint>

namespace rm_decision
{

CheckDefendCondition::CheckDefendCondition(const std::string & n, const BT::NodeConfig & c)
    : ConditionNode(n, c)
{
}
BT::PortsList CheckDefendCondition::providedPorts()
{
    return {BT::InputPort<float>("home_hp_threshold", 300.f, "")};
}
BT::NodeStatus CheckDefendCondition::tick()
{
    float thr = 300.f;
    (void)getInput("home_hp_threshold", thr);
    float base = 0;
    (void)config().blackboard->get(BbKey::kAllyBaseHp, base);
    return base > 0.f && base < thr ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
}

CheckAttackCondition::CheckAttackCondition(const std::string & n, const BT::NodeConfig & c)
    : ConditionNode(n, c)
{
}
BT::NodeStatus CheckAttackCondition::tick()
{
    std::uint8_t st = 0;
    int t = 0;
    std::uint8_t progress = 0;
    (void)config().blackboard->get(BbKey::kBigEnergyStatus, st);
    (void)config().blackboard->get(BbKey::kGameTimeRemaining, t);
    (void)config().blackboard->get(BbKey::kGameProgress, progress);
    if (progress == 4u && st != 0 && t <= 120)
        return BT::NodeStatus::SUCCESS;
    return BT::NodeStatus::FAILURE;
}

} // namespace rm_decision
