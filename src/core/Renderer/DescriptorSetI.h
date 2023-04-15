// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

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

    void addSampler(ShaderStage stage, const TextureRef texture) { static_cast<T*>(this)->addSampler(stage, texture); }

    template <class Tx> void setUniform(entt::hashed_string::hash_type id, const Tx& data)
    {
        static_cast<T*>(this)->setUniform<Tx>(id, data);
    }

    void build() { static_cast<T*>(this)->build(); }

    [[nodiscard]] static DescriptorSetRef create() { return T::create(); }

private:
    DescriptorSetI() = default;
    friend T;
};

} // namespace chronicle