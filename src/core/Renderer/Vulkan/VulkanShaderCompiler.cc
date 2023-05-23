// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanShaderCompiler.h"

#include "Storage/Storage.h"
#include "VulkanCommon.h"
#include "VulkanShader.h"

namespace chronicle {

struct VulkanShaderFileInfo {
    std::string content;
};

class VulkanShaderIncluder : public shaderc::CompileOptions::IncluderInterface {
public:
    shaderc_include_result* GetInclude(const char* requested_source, shaderc_include_type type,
        const char* requesting_source, size_t include_depth) override
    {
        std::string filename
            = type == shaderc_include_type_standard ? fmt::format(":/{}", requested_source) : requested_source;

        if (!Storage::exists(filename))
            return makeErrorIncludeResult("Cannot find or open include file.");

        try {
            auto content = Storage::readString(filename);
            auto info = new VulkanShaderFileInfo();
            info->content = std::move(content);
            return new shaderc_include_result { requested_source, std::strlen(requested_source), info->content.data(),
                info->content.size(), info };
        } catch (const StorageError& ex) {
            return makeErrorIncludeResult(ex.what());
        }
    }

    void ReleaseInclude(shaderc_include_result* data) override
    {
        if (data->user_data != nullptr)
            delete data->user_data;
    }

private:
    shaderc_include_result* makeErrorIncludeResult(const char* message)
    {
        return new shaderc_include_result { "", 0, message, strlen(message) };
    }
};

ShaderRef VulkanShaderCompiler::compile(const ShaderCompilerOptions& options)
{
    assert(!options.filename.empty());

    auto shaderLanguage = detectShaderLanguage(options.filename);
    if (!shaderLanguage)
        throw RendererError(fmt::format("Unsupport shader language for file {}.", options.filename));

    auto sourceCode = Storage::readString(options.filename);
    assert(!sourceCode.empty());

    std::unordered_map<ShaderStage, std::vector<uint8_t>> codes;
    std::unordered_map<ShaderStage, std::string> entryPoints;

    std::array<ShaderStage, 2> stages = { ShaderStage::vertex, ShaderStage::fragment };

    for (const auto stage : stages) {
        codes[stage] = compile(sourceCode, shaderLanguage.value(), options, stage, "main");
        entryPoints[stage] = "main";
    }

    return VulkanShader::create(codes, entryPoints, std::hash<ShaderCompilerOptions>()(options));
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

std::optional<shaderc_source_language> VulkanShaderCompiler::detectShaderLanguage(const std::string_view& filename)
{
    if (filename.ends_with(".hlsl"))
        return shaderc_source_language_hlsl;
    if (filename.ends_with(".glsl"))
        return shaderc_source_language_glsl;
    return std::optional<shaderc_source_language>();
}

ShaderStage VulkanShaderCompiler::detectPragmaStage(const std::string& line)
{
    auto regex = std::regex("#pragma stage:(.*)");
    std::smatch match;
    if (std::regex_search(line, match, regex)) {
        if (match[1] == "all")
            return ShaderStage::all;
        else if (match[1] == "fragment")
            return ShaderStage::fragment;
        else if (match[1] == "vertex")
            return ShaderStage::vertex;
        else if (match[1] == "compute")
            return ShaderStage::compute;

        CHRLOG_ERROR("Invalid shader stage {}", match[1].str());
    }
    return ShaderStage::none;
}

std::string VulkanShaderCompiler::cleanSourceFromOtherStages(const std::string& source, ShaderStage shaderStage)
{
    ShaderStage currentShaderStage = ShaderStage::all;

    std::string result {};
    std::stringstream stream(source);
    std::string line;
    while (std::getline(stream, line, '\n')) {
        auto lineStage = detectPragmaStage(line);
        if (lineStage != ShaderStage::none) {
            currentShaderStage = lineStage;
            result.append("\n");
            continue;
        }

        if (!!(currentShaderStage & shaderStage)) {
            result.append(line);
        }
        result.append("\n");
    }
    return result;
}

std::vector<uint8_t> VulkanShaderCompiler::compile(const std::string_view& sourceCode,
    shaderc_source_language shaderLanguage, const ShaderCompilerOptions& options, ShaderStage shaderStage,
    const std::string& entryPoint)
{
    shaderc::Compiler spirvCompiler = {};
    shaderc::CompileOptions spirvOptions = {};

    spirvOptions.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_1);
#ifdef NDEBUG
    spirvOptions.SetOptimizationLevel(shaderc_optimization_level_performance);
#else
    spirvOptions.SetOptimizationLevel(shaderc_optimization_level_zero);
    spirvOptions.SetGenerateDebugInfo();
#endif
    spirvOptions.SetSourceLanguage(shaderLanguage);
    spirvOptions.SetIncluder(std::make_unique<VulkanShaderIncluder>());

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

    for (const auto& macrodDefition : options.macroDefinitions) {
        spirvOptions.AddMacroDefinition(macrodDefition);
    }

    auto preprocessResult = spirvCompiler.PreprocessGlsl(
        sourceCode.data(), sourceCode.size(), getSpirvShader(shaderStage), options.filename.c_str(), spirvOptions);
    auto preprocessContent = std::string { preprocessResult.cbegin(), preprocessResult.cend() };

    preprocessContent = cleanSourceFromOtherStages(preprocessContent, shaderStage);

    auto spirvCompilerResult = spirvCompiler.CompileGlslToSpv(
        preprocessContent, getSpirvShader(shaderStage), options.filename.c_str(), entryPoint.c_str(), spirvOptions);

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
            fmt::format("Can't compile shader {} for stage {}", options.filename, magic_enum::enum_name(shaderStage)));

    auto data = std::vector<uint8_t>(((spirvCompilerResult.end() - spirvCompilerResult.begin()) * sizeof(uint32_t)));
    std::memcpy(data.data(), spirvCompilerResult.begin(), data.size());
    return data;
}

} // namespace chronicle