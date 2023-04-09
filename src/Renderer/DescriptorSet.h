#pragma once

#include "pch.h"

#include "Common.h"

namespace chronicle {

template <class T> class DescriptorSetI {
public:
    template <class Tx> void addUniform(entt::hashed_string::hash_type id, ShaderStage stage)
    {
        static_cast<T*>(this)->addUniform<Tx>(id, stage);
    }

    void addSampler(ShaderStage stage, const std::shared_ptr<Image> image)
    {
        static_cast<T*>(this)->addSampler(stage, image);
    }

    template <class Tx> void setUniform(entt::hashed_string::hash_type id, const Tx& data)
    {
        static_cast<T*>(this)->setUniform<Tx>(id, data);
    }

    void build() { static_cast<T*>(this)->build(); }

    static DescriptorSetRef create(const Renderer* renderer) { return T::create(renderer); }

private:
    DescriptorSetI() = default;
    friend T;
};

} // namespace chronicle