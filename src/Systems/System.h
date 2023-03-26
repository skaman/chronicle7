#pragma once

#include <entt/entt.hpp>

namespace chronicle {

class System {
public:
    virtual ~System() = default;
    virtual void Run(entt::registry& registry) = 0;
};

} // namespace chronicle