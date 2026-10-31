#pragma once

#include <behaviortree_cpp/condition_node.h>

#include <string>

namespace rm_decision
{

class CheckHeat : public BT::ConditionNode {
public:
    CheckHeat(const std::string & _name, const BT::NodeConfig & _config);
    static BT::PortsList providedPorts();
    BT::NodeStatus tick() override;
};

class CheckEngagedStatus : public BT::ConditionNode {
public:
    CheckEngagedStatus(const std::string & _name, const BT::NodeConfig & _config);
    static BT::PortsList providedPorts();
    BT::NodeStatus tick() override;
};

class CheckOutpostTarget : public BT::ConditionNode {
public:
    CheckOutpostTarget(const std::string & _name, const BT::NodeConfig & _config);
    static BT::PortsList providedPorts();
    BT::NodeStatus tick() override;
};

class CheckOutpostLowHealthDefend : public BT::ConditionNode {
public:
    CheckOutpostLowHealthDefend(const std::string & _name, const BT::NodeConfig & _config);
    static BT::PortsList providedPorts();
    BT::NodeStatus tick() override;
};

class CheckEnemyAreaRecentlyHurt : public BT::ConditionNode {
public:
    CheckEnemyAreaRecentlyHurt(const std::string & _name, const BT::NodeConfig & _config);
    static BT::PortsList providedPorts();
    BT::NodeStatus tick() override;
};

class CheckEnemyDefenseHealthDrop : public BT::ConditionNode {
public:
    CheckEnemyDefenseHealthDrop(const std::string & _name, const BT::NodeConfig & _config);
    static BT::PortsList providedPorts();
    BT::NodeStatus tick() override;
};

} // namespace rm_decision
