#pragma once

#include "pch.h"

namespace chronicle {

class System {
public:
    virtual ~System() = default;
    virtual void run(entt::registry& registry) = 0;
};

} // namespace chronicle