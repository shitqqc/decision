#pragma once

#include "rm_decision/domain/zone_map.hpp"

#include <behaviortree_cpp/action_node.h>

#include <string>

namespace rm_decision
{

class SetCoordinate : public BT::SyncActionNode {
public:
    SetCoordinate(const std::string & _name, const BT::NodeConfig & _config, const ZoneMap * _zones);
    static BT::PortsList providedPorts();
    BT::NodeStatus tick() override;

private:
    const ZoneMap * zones_;
};

class SetNavMode : public BT::SyncActionNode {
public:
    SetNavMode(const std::string & _name, const BT::NodeConfig & _config);
    static BT::PortsList providedPorts();
    BT::NodeStatus tick() override;
};

class SetManualNavGoal : public BT::SyncActionNode {
public:
    SetManualNavGoal(const std::string & _name, const BT::NodeConfig & _config);
    static BT::PortsList providedPorts();
    BT::NodeStatus tick() override;
};

/// Alias of bit SetManualOverrideGoal.
using SetManualOverrideGoal = SetManualNavGoal;

class SelectPatrolPoint : public BT::SyncActionNode {
public:
    SelectPatrolPoint(const std::string & _name, const BT::NodeConfig & _config, const ZoneMap * _zones);
    static BT::PortsList providedPorts();
    BT::NodeStatus tick() override;

private:
    const ZoneMap * zones_;
    int patrol_index_{0};
};

class Wait : public BT::StatefulActionNode {
public:
    Wait(const std::string & _name, const BT::NodeConfig & _config);
    static BT::PortsList providedPorts();
    BT::NodeStatus onStart() override;
    BT::NodeStatus onRunning() override;
    void onHalted() override {}

private:
    double end_sec_{0.0};
};

// class SetStairsPosition : public BT::SyncActionNode
// {
// public:
//   SetStairsPosition(
//     const std::string & _name, const BT::NodeConfig & _config, const ZoneMap * _zones);
//   static BT::PortsList providedPorts();
//   BT::NodeStatus tick() override;

// private:
//   const ZoneMap * zones_;
// };

class AccumulateAmmoPurchase : public BT::SyncActionNode {
public:
    AccumulateAmmoPurchase(const std::string & _name, const BT::NodeConfig & _config);
    static BT::PortsList providedPorts();
    BT::NodeStatus tick() override;

private:
    double last_purchase_sec_{-1.0e9};
};

} // namespace rm_decision
