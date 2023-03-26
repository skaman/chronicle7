#pragma once

#include "System.h"

namespace chronicle {

class MeshRendererSystem : public System {
public:
    explicit MeshRendererSystem();
    ~MeshRendererSystem() override = default;
    void Run(entt::registry& registry) override;
};

} // namespace chronicle