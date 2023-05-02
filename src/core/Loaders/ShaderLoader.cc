// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "ShaderLoader.h"

namespace chronicle {

struct ShaderLoaderContext {
    static inline std::unordered_map<size_t, std::weak_ptr<Shader>> cache = {};
};

ShaderRef ShaderLoader::load(const std::string& filename)
{
    auto hash = std::hash<std::string>()(filename);
    if (auto shader = ShaderLoaderContext::cache[hash].lock()) {
        return shader;
    }

    auto shader = ShaderCompiler::compile(filename);
    ShaderLoaderContext::cache[hash] = shader;
    return shader;
}

} // namespace chronicle