#pragma once

#include <memory>
#include <vector>

#include <entt/entt.hpp>

#include "System.h"

namespace chronicle {

class Systems {
public:
    template <typename Impl> inline void Register() { _systems.push_back(std::make_unique<Impl>()); }

    void Run(entt::registry& registry)
    {
        std::ranges::for_each(_systems, [&registry](const std::unique_ptr<System>& system) { system->Run(registry); });
    }

private:
    std::vector<std::unique_ptr<System>> _systems;
};

} // namespace chronicle