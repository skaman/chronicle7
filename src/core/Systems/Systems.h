#pragma once

#include "pch.h"

#include "System.h"

namespace chronicle {

class Systems {
public:
    template <typename Impl> void add() { _systems.push_back(std::make_unique<Impl>()); }

    void run(entt::registry& registry)
    {
        std::ranges::for_each(_systems, [&registry](const std::unique_ptr<System>& system) { system->run(registry); });
    }

private:
    std::vector<std::unique_ptr<System>> _systems;
};

using SystemsUnique = std::unique_ptr<Systems>;

} // namespace chronicle