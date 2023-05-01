// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanShaderCompiler.h"

#include "Storage/Storage.h"
#include "VulkanCommon.h"
#include "VulkanShader.h"

namespace chronicle {

ShaderRef VulkanShaderCompiler::compile(const std::string& filename, ShaderCompilerOptions options)
{
    auto sourceCode = Storage::readString(filename);

    std::unordered_map<ShaderStage, std::vector<uint8_t>> codes;
    std::unordered_map<ShaderStage, std::string> entryPoints;

    magic_enum::enum_for_each<ShaderStage>([&codes, &entryPoints, &options, &filename, &sourceCode](auto val) {
        constexpr ShaderStage shaderStage = val;

        if (shaderStage == ShaderStage::none || shaderStage == ShaderStage::_entt_enum_as_bitmask) {
            return;
        }

        auto entryPoint = getEntryPoint(sourceCode, shaderStage);
        if (entryPoint.empty()) {
            return;
        }

        codes[shaderStage] = compile(sourceCode, filename, shaderStage, entryPoint, options);
        entryPoints[shaderStage] = entryPoint;
    });

    return VulkanShader::create(codes, entryPoints);
}

shaderc_shader_kind VulkanShaderCompiler::getSpirvShader(ShaderStage stage)
{
    switch (stage) {
    case ShaderStage::vertex:
        return shaderc_vertex_shader;
    case ShaderStage::fragment:
        return shaderc_fragment_shader;
    case ShaderStage::compute:
        return shaderc_compute_shader;
    default:
        throw RendererError("Unsupported shader stage");
    }
}

shaderc_source_language VulkanShaderCompiler::getSpirvLanguage(ShaderSourceLanguage language)
{
    switch (language) {
    case ShaderSourceLanguage::glsl:
        return shaderc_source_language_glsl;
    case ShaderSourceLanguage::hlsl:
        return shaderc_source_language_hlsl;
    default:
        throw RendererError("Unsupported language type");
    }
}

shaderc_optimization_level VulkanShaderCompiler::getSpirvOptimizationLevel(ShaderOptimizationLevel optimizationLevel)
{
    switch (optimizationLevel) {
    case ShaderOptimizationLevel::none:
        return shaderc_optimization_level_zero;
    case ShaderOptimizationLevel::size:
        return shaderc_optimization_level_size;
    case ShaderOptimizationLevel::performance:
        return shaderc_optimization_level_performance;
    default:
        throw RendererError("Unsupported optimization level");
    }
}

std::string VulkanShaderCompiler::getEntryPoint(const std::string& sourceCode, ShaderStage shaderStage)
{
    std::regex regex;

    switch (shaderStage) {
    case ShaderStage::fragment:
        regex = std::regex(".*#pragma fragment (.*)");
        break;
    case ShaderStage::vertex:
        regex = std::regex(".*#pragma vertex (.*)");
        break;
    case ShaderStage::compute:
        regex = std::regex(".*#pragma compute (.*)");
        break;
    default:
        return {};
    }

    std::stringstream stream(sourceCode);
    std::string line;
    std::smatch match;
    while (std::getline(stream, line, '\n')) {
        if (std::regex_search(line, match, regex)) {
            return match[1];
        }
    }

    return {};
}

std::vector<uint8_t> VulkanShaderCompiler::compile(const std::string_view& sourceCode, const std::string& filename,
    ShaderStage shaderStage, const std::string& entryPoint, ShaderCompilerOptions options)
{
    shaderc::Compiler spirvCompiler = {};
    shaderc::CompileOptions spirvOptions = {};

    spirvOptions.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_1);
    spirvOptions.SetOptimizationLevel(getSpirvOptimizationLevel(options.optimization));
    spirvOptions.SetSourceLanguage(getSpirvLanguage(options.language));

    if (options.warningAsErrors) {
        spirvOptions.SetWarningsAsErrors();
    }

    switch (shaderStage) {
    case ShaderStage::fragment:
        spirvOptions.AddMacroDefinition("IS_FRAGMENT");
        break;
    case ShaderStage::vertex:
        spirvOptions.AddMacroDefinition("IS_VERTEX");
        break;
    case ShaderStage::compute:
        spirvOptions.AddMacroDefinition("IS_COMPUTE");
        break;
    default:
        break;
    }

    // sprivCompiler.PreprocessGlsl
    auto spirvCompilerResult = spirvCompiler.CompileGlslToSpv(sourceCode.data(), sourceCode.size(),
        getSpirvShader(shaderStage), filename.c_str(), entryPoint.c_str(), spirvOptions);

    std::stringstream streamMessage(spirvCompilerResult.GetErrorMessage());
    std::string segment;
    const std::regex errorRegex(".*:\\d+: error:.*");
    const std::regex warningRegex(".*:\\d+: warning:.*");

    while (std::getline(streamMessage, segment, '\n')) {
        if (std::regex_match(segment, errorRegex)) {
            CHRLOG_ERROR("{}", segment);
        } else if (std::regex_match(segment, warningRegex)) {
            CHRLOG_WARN("{}", segment);
        } else {
            CHRLOG_INFO("{}", segment);
        }
    }

    if (spirvCompilerResult.GetCompilationStatus() != shaderc_compilation_status_success)
        throw RendererError(
            fmt::format("Can't compile shader {} for stage {}", filename, magic_enum::enum_name(shaderStage)));

    auto data = std::vector<uint8_t>(((spirvCompilerResult.end() - spirvCompilerResult.begin()) * sizeof(uint32_t)));
    std::memcpy(data.data(), spirvCompilerResult.begin(), data.size());
    return data;
}

} // namespace chronicle