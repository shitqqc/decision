#include "rm_decision/bt/action/change_stance_action.hpp"
#include "rm_decision/domain/blackboard_keys.hpp"
#include <chrono>
#include <cstdint>

namespace rm_decision
{

static std::uint8_t parseStance(const std::string & s)
{
    if (s == "ATTACK" || s == "attack")
        return static_cast<std::uint8_t>(StanceCmd_e::Attack);
    if (s == "DEFEND" || s == "defend")
        return static_cast<std::uint8_t>(StanceCmd_e::Defend);
    if (s == "ENHANCED_ATTACK")
        return static_cast<std::uint8_t>(StanceCmd_e::EnhancedAttack);
    if (s == "ENHANCED_DEFEND")
        return static_cast<std::uint8_t>(StanceCmd_e::EnhancedDefend);
    if (s == "ENHANCED_MOVE")
        return static_cast<std::uint8_t>(StanceCmd_e::EnhancedMove);
    return static_cast<std::uint8_t>(StanceCmd_e::Move);
}

SetSpinState::SetSpinState(const std::string & n, const BT::NodeConfig & c) : SyncActionNode(n, c)
{
}
BT::PortsList SetSpinState::providedPorts()
{
    return {
        BT::InputPort<bool>("use_spin", false, "")
    };
}
BT::NodeStatus SetSpinState::tick()
{
    bool use = false;
    (void)getInput("use_spin", use);
    config().blackboard->set(BbKey::kUseSpin, use);
    return BT::NodeStatus::SUCCESS;
}

SetDesiredStance::SetDesiredStance(const std::string & n, const BT::NodeConfig & c)
    : SyncActionNode(n, c)
{
}
BT::PortsList SetDesiredStance::providedPorts()
{
    return {BT::InputPort<std::string>("stance", "MOVE", "")};
}
BT::NodeStatus SetDesiredStance::tick()
{
    std::string s = "MOVE";
    (void)getInput("stance", s);
    config().blackboard->set(BbKey::kDesiredStance, parseStance(s));
    return BT::NodeStatus::SUCCESS;
}

SetSuperCap::SetSuperCap(const std::string & n, const BT::NodeConfig & c) : SyncActionNode(n, c) {}
BT::PortsList SetSuperCap::providedPorts() { return {BT::InputPort<bool>("enable", false, "")}; }
BT::NodeStatus SetSuperCap::tick()
{
    bool en = false;
    (void)getInput("enable", en);
    config().blackboard->set(BbKey::kUseSuperCap, en);
    return BT::NodeStatus::SUCCESS;
}

ChangeStance::ChangeStance(const std::string & n, const BT::NodeConfig & c)
    : StatefulActionNode(n, c)
{
}
BT::PortsList ChangeStance::providedPorts()
{
    return {
        BT::InputPort<std::string>("stance", "MOVE", ""),
        BT::InputPort<double>("cooldown", 5.0, "")};
}
BT::NodeStatus ChangeStance::onStart()
{
    double cd = 5.0;
    (void)getInput("cooldown", cd);
    const auto now = std::chrono::steady_clock::now().time_since_epoch();
    const double t = std::chrono::duration_cast<std::chrono::duration<double>>(now).count();
    if (last_change_sec_ > 0.0 && (t - last_change_sec_) < cd)
        return BT::NodeStatus::SUCCESS;

    std::string s = "MOVE";
    (void)getInput("stance", s);
    config().blackboard->set(BbKey::kDesiredStance, parseStance(s));
    last_change_sec_ = t;
    return BT::NodeStatus::RUNNING;
}
BT::NodeStatus ChangeStance::onRunning()
{
    // Wait until posture feedback matches if available; else succeed immediately
    std::uint8_t desired = 3;
    std::uint8_t feedback = 0;
    (void)config().blackboard->get(BbKey::kDesiredStance, desired);
    (void)config().blackboard->get(BbKey::kPostureFeedback, feedback);
    if (feedback == 0) {
        (void)config().blackboard->get(BbKey::kCurrentStance, feedback);
    }
    if (feedback == 0 || feedback == desired) {
        config().blackboard->set(BbKey::kCurrentStance, desired);
        return BT::NodeStatus::SUCCESS;
    }
    return BT::NodeStatus::RUNNING;
}

UpdateStanceDuration::UpdateStanceDuration(const std::string & n, const BT::NodeConfig & c)
    : SyncActionNode(n, c)
{
}
BT::NodeStatus UpdateStanceDuration::tick() 
{ 
    int current_time = 0;
    std::uint8_t game_progress = 0;
    (void)config().blackboard->get(BbKey::kGameTimeRemaining, current_time);
    (void)config().blackboard->get(BbKey::kGameProgress, game_progress);
    const int max_game_time = 120;
    const bool game_active =
        (game_progress == 4) || (current_time > 0 && current_time <= max_game_time);
    if (last_game_time_ < 0) {
        last_game_time_ = current_time;
        return BT::NodeStatus::SUCCESS;
    }
    if (!game_active) {
        last_game_time_ = current_time;
        return BT::NodeStatus::SUCCESS;
    }

    const int raw_delta = last_game_time_ - current_time;
    last_game_time_ = current_time;
    const int max_delta = 5;
    if (raw_delta <= 0 || raw_delta > max_delta) {
        return BT::NodeStatus::SUCCESS;
    }

    std::uint8_t current_stance = 0;
    (void)config().blackboard->get(BbKey::kCurrentStance, current_stance);
    switch (current_stance) {
    case static_cast<std::uint8_t>(StanceCmd_e::Attack):
    case static_cast<std::uint8_t>(StanceCmd_e::EnhancedAttack): {
        float value = 0.0f;
        (void)config().blackboard->get(BbKey::kEnhancedAttackRemaining, value);
        break;
    }
    case static_cast<std::uint8_t>(StanceCmd_e::Defend):
    case static_cast<std::uint8_t>(StanceCmd_e::EnhancedDefend): {
        float value = 0.0f;
        (void)config().blackboard->get(BbKey::kEnhancedDefendRemaining, value);
        break;
    }
    case static_cast<std::uint8_t>(StanceCmd_e::Move):
    case static_cast<std::uint8_t>(StanceCmd_e::EnhancedMove): {
        float value = 0.0f;
        (void)config().blackboard->get(BbKey::kEnhancedMoveRemaining, value);
        break;
    }
    default:
        break;
    }

    return BT::NodeStatus::SUCCESS;
}

ApplyManualStanceOverride::ApplyManualStanceOverride(const std::string & n, const BT::NodeConfig & c)
    : SyncActionNode(n, c)
{
}
BT::NodeStatus ApplyManualStanceOverride::tick()
{
    bool active = false;
    (void)config().blackboard->get(BbKey::kManualStanceOverrideActive, active);
    if (!active)
        return BT::NodeStatus::FAILURE;
    std::uint8_t st = 3;
    (void)config().blackboard->get(BbKey::kManualStanceOverride, st);
    config().blackboard->set(BbKey::kDesiredStance, st);
    return BT::NodeStatus::SUCCESS;
}

} // namespace rm_decision
