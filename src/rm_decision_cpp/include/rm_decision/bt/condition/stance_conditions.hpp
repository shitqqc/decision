#pragma once

#include <behaviortree_cpp/condition_node.h>

#include <string>

namespace rm_decision
{

class CheckCapacitorCapacity : public BT::ConditionNode {
public:
    CheckCapacitorCapacity(const std::string & _name, const BT::NodeConfig & _config);
    static BT::PortsList providedPorts();
    BT::NodeStatus tick() override;
};

class CheckStanceEffectLimit : public BT::ConditionNode {
public:
    CheckStanceEffectLimit(const std::string & _name, const BT::NodeConfig & _config);
    static BT::PortsList providedPorts();
    BT::NodeStatus tick() override;
};

class CheckStanceRefreshRequired : public BT::ConditionNode {
public:
    CheckStanceRefreshRequired(const std::string & _name, const BT::NodeConfig & _config);
    static BT::PortsList providedPorts();
    BT::NodeStatus tick() override;
};

class CheckShouldEnhanceStance : public BT::ConditionNode {
public:
    CheckShouldEnhanceStance(const std::string & _name, const BT::NodeConfig & _config);
    static BT::PortsList providedPorts();
    BT::NodeStatus tick() override;
};

} // namespace rm_decision
