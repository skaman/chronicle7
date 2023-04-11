#pragma once

#include "pch.h"

namespace chronicle {

class System {
public:
    virtual ~System() = default;
    virtual void activate() = 0;
    virtual void deactivate() = 0;
    virtual void run(entt::registry& registry) = 0;
};

} // namespace chronicle