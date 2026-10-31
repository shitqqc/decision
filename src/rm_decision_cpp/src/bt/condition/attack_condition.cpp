#include "rm_decision/bt/condition/attack_condition.hpp"
#include "rm_decision/domain/blackboard_keys.hpp"
#include <cstdint>

namespace rm_decision
{

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
