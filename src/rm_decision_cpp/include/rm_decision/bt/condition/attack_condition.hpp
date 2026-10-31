#pragma once

#include "rm_decision/domain/zone_map.hpp"

#include <behaviortree_cpp/condition_node.h>

#include <string>

namespace rm_decision
{

class CheckAttackCondition : public BT::ConditionNode {
public:
    CheckAttackCondition(const std::string & _name, const BT::NodeConfig & _config);
    static BT::PortsList providedPorts() { return {}; }
    BT::NodeStatus tick() override;
};

} // namespace rm_decision
