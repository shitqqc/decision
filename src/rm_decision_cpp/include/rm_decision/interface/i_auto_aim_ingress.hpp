#pragma once

#include <behaviortree_cpp/blackboard.h>

namespace rm_decision
{

class IAutoAimIngress {
public:
    virtual ~IAutoAimIngress() = default;

    virtual void update(BT::Blackboard & _bb) = 0;
};

}
