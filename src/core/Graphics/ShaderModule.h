// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "Common.h"

namespace chronicle::graphics
{

/// @brief Error exception for invalid shader module operations.
class ShaderModuleError : public GraphicsError
{
  public:
    explicit ShaderModuleError(const std::string &message) : GraphicsError(message.c_str())
    {
    }
};

/// @brief Description for shader module creation.
struct ShaderModuleDescriptor
{
    /// @brief Shader module name.
    std::string name{};

    /// @brief Shader source code.
    std::string sourceCode{};

    /// @brief Shader stage.
    ShaderStage shaderStage{};

    /// @brief Shader compiler optimization level.
    ShaderOptimizationLevel optimizationLevel{ShaderOptimizationLevel::ePerformance};

    /// @brief Macro definitions used to compile the shader.
    std::vector<std::string> macroDefinitions{};
};

/// @brief A ShaderModule is a reference to an internal shader module object.
class ShaderModule
{
  public:
    /// @brief Constructor.
    /// @param shaderModuleDescriptor Shader module descriptor.
    explicit ShaderModule(const ShaderModuleDescriptor &shaderModuleDescriptor)
        : _shaderModuleDescriptor(shaderModuleDescriptor)
    {
    }

    /// @brief Destructor.
    virtual ~ShaderModule() = default;

    /// @brief Returns the descriptor used to create the shader module.
    /// @return Shader module descriptor.
    const ShaderModuleDescriptor &descriptor() const
    {
        return _shaderModuleDescriptor;
    }

  private:
    ShaderModuleDescriptor _shaderModuleDescriptor; ///< Shader module descriptor.
};

} // namespace chronicle::graphics
