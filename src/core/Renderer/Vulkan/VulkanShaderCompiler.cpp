// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanShaderCompiler.h"

#include "Storage/StorageContext.h"
#include "VulkanCommon.h"
#include "VulkanShader.h"

namespace chronicle::internal::vulkan {

struct VulkanShaderFileInfo {
    std::string content;
};

class VulkanShaderIncluder : public shaderc::CompileOptions::IncluderInterface {
public:
    shaderc_include_result* GetInclude(const char* requested_source, shaderc_include_type type,
        const char* requesting_source, size_t include_depth) override
    {
        if (!StorageContext::exists(requested_source))
            return makeErrorIncludeResult("Cannot find or open include file.");

        try {
            auto file = StorageContext::file(requested_source);
            auto content = file.readAllText();
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

ShaderCompilerResult VulkanShaderCompiler::compile(const ShaderCompilerOptions& options)
{
    assert(!options.filename.empty());

    auto shaderLanguage = detectShaderLanguage(options.filename);
    if (!shaderLanguage)
        throw RendererError(fmt::format("Unsupport shader language for file {}.", options.filename));

    auto file = StorageContext::file(options.filename);
    auto sourceCode = file.readAllText();
    assert(!sourceCode.empty());

    std::unordered_map<ShaderStage, std::vector<uint8_t>> codes;
    std::unordered_map<ShaderStage, std::string> entryPoints;

    std::array<ShaderStage, 2> stages = { ShaderStage::vertex, ShaderStage::fragment };

    ShaderCompilerResult result {};
    result.modules.reserve(stages.size());
    for (const auto stage : stages) {
        result.modules[stage]
            = compileModule(sourceCode, shaderLanguage.value(), options, stage, result.descriptorSetsLayout);
    }

    return result;
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

        throw RendererError(fmt::format("Invalid shader stage {}", match[1].str()));
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

ShaderCompilerModule VulkanShaderCompiler::compileModule(const std::string_view& sourceCode,
    shaderc_source_language shaderLanguage, const ShaderCompilerOptions& options, ShaderStage shaderStage,
    std::array<DescriptorSetLayout, MaxDescriptorSetsCount>& descriptorSetsLayout)
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

    // preprocess
    auto preprocessResult = spirvCompiler.PreprocessGlsl(
        sourceCode.data(), sourceCode.size(), getSpirvShader(shaderStage), options.filename.c_str(), spirvOptions);
    auto preprocessContent = std::string { preprocessResult.cbegin(), preprocessResult.cend() };
    preprocessContent = cleanSourceFromOtherStages(preprocessContent, shaderStage);

    // compile
    auto spirvCompilerResult = spirvCompiler.CompileGlslToSpv(
        preprocessContent, getSpirvShader(shaderStage), options.filename.c_str(), spirvOptions);

    // parse compiler results
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

    // get the spir-v binary
    auto spirvBinary = std::vector<uint32_t>(spirvCompilerResult.begin(), spirvCompilerResult.end());

    // reflection
    spirv_cross::Compiler spirvCrossCompiler(spirvBinary.data(), spirvBinary.size());

    const auto& entryPointsAndStages = spirvCrossCompiler.get_entry_points_and_stages();
    if (entryPointsAndStages.size() != 1) {
        throw RendererError(fmt::format("Only 1 entrypoint for shader is supported. Found {} entrypoints in shader {}",
            entryPointsAndStages.size(), options.filename));
    }

    for (auto i = 0; i < MaxDescriptorSetsCount; i++) {
        descriptorSetsLayout[i].setNumber = i;
    }

    // reflect resources
    auto resources = spirvCrossCompiler.get_shader_resources();
    for (const auto& resource : resources.uniform_buffers) {
        if (!isResourceInUse(spirvCrossCompiler, resource)) {
            continue;
        }

        auto binding
            = parseResource(spirvCrossCompiler, resource, options.filename, shaderStage, DescriptorType::uniformBuffer);
        addBindingToDescriptorSet(descriptorSetsLayout, binding);
    }

    for (const auto& resource : resources.sampled_images) {
        auto binding = parseResource(
            spirvCrossCompiler, resource, options.filename, shaderStage, DescriptorType::combinedImageSampler);
        addBindingToDescriptorSet(descriptorSetsLayout, binding);
    }

    return ShaderCompilerModule { .spirvBinary = spirvBinary, .entryPoint = entryPointsAndStages[0].name };
}

DescriptorSetLayoutBinding VulkanShaderCompiler::parseResource(const spirv_cross::Compiler& compiler,
    const spirv_cross::Resource& resource, const std::string& filename, ShaderStage shaderStage,
    DescriptorType descriptorType)
{
    auto& bufferType = compiler.get_type(resource.base_type_id);

    DescriptorSetLayoutBinding binding {};
    binding.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
    binding.descriptorSet = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
    binding.descriptorType = descriptorType;
    binding.name = resource.name;
    if (descriptorType == DescriptorType::uniformBuffer) {
        binding.uniformSize = compiler.get_declared_struct_size(bufferType);
        binding.uniformMembers.resize(bufferType.member_types.size());
        for (auto i = 0; i < binding.uniformMembers.size(); i++) {
            auto& member = binding.uniformMembers[i];
            const auto& memberType = compiler.get_type(bufferType.member_types[i]);
            member.type = typeFromSpirType(filename, memberType);
            member.name = compiler.get_member_name(resource.base_type_id, i);
            member.arraySize = memberType.array.empty() ? 1 : memberType.array[0];
        }
    }
    binding.arraySize = bufferType.array.empty() ? 1 : bufferType.array[0];
    binding.stages |= shaderStage;

    return binding;
}

bool VulkanShaderCompiler::isResourceInUse(const spirv_cross::Compiler& compiler, const spirv_cross::Resource& resource)
{
    return !compiler.get_active_buffer_ranges(resource.id).empty();
}

void VulkanShaderCompiler::addBindingToDescriptorSet(
    std::array<DescriptorSetLayout, MaxDescriptorSetsCount>& descriptorSetsLayout,
    const DescriptorSetLayoutBinding& binding)
{
    if (binding.descriptorSet >= MaxDescriptorSetsCount) {
        throw RendererError(fmt::format("Only first {} descriptor sets are supported. Descriptor set {} is invalid.",
            MaxDescriptorSetsCount, binding.descriptorSet));
    }

    auto& descriptorSet = descriptorSetsLayout[binding.descriptorSet];

    if (descriptorSet.bindings.contains(binding.binding)) {
        auto& otherBinding = descriptorSet.bindings[binding.binding];
        if (binding.checkCompatibility(otherBinding)) {
            otherBinding.stages |= binding.stages;
        } else {
            throw RendererError(fmt::format("Incompatible descriptor between stages: set = {} binding = {}",
                binding.descriptorSet, binding.binding));
        }
    } else {
        descriptorSet.bindings[binding.binding] = binding;
    }
}

Type VulkanShaderCompiler::typeFromSpirType(const std::string& filename, const spirv_cross::SPIRType& type)
{
    if (type.basetype == spirv_cross::SPIRType::BaseType::Float) {
        if (type.columns == 1) {
            switch (type.vecsize) {
            case 1:
                return Type::scalarFloat;
            case 2:
                return Type::vectorFloat2;
            case 3:
                return Type::vectorFloat3;
            case 4:
                return Type::vectorFloat4;
            default:
                break;
            }
        } else if (type.columns == 2) {
            switch (type.vecsize) {
            case 2:
                return Type::matrix2x2;
            case 3:
                return Type::matrix2x3;
            case 4:
                return Type::matrix2x4;
            default:
                break;
            }
        } else if (type.columns == 3) {
            switch (type.vecsize) {
            case 2:
                return Type::matrix3x2;
            case 3:
                return Type::matrix3x3;
            case 4:
                return Type::matrix3x4;
            default:
                break;
            }
        } else if (type.columns == 4) {
            switch (type.vecsize) {
            case 2:
                return Type::matrix4x2;
            case 3:
                return Type::matrix4x3;
            case 4:
                return Type::matrix4x4;
            default:
                break;
            }
        }
    } else if (type.basetype == spirv_cross::SPIRType::BaseType::Boolean && type.vecsize == 1 && type.columns == 1) {
        return Type::scalarBool;
    } else if (type.basetype == spirv_cross::SPIRType::BaseType::Int && type.columns == 1) {
        switch (type.vecsize) {
        case 1:
            return Type::scalarInt;
        case 2:
            return Type::vectorInt2;
        case 3:
            return Type::vectorInt3;
        case 4:
            return Type::vectorInt4;
        default:
            break;
        }
    } else if (type.basetype == spirv_cross::SPIRType::BaseType::UInt && type.columns == 1) {
        switch (type.vecsize) {
        case 1:
            return Type::scalarUInt;
        case 2:
            return Type::vectorUInt2;
        case 3:
            return Type::vectorUInt3;
        case 4:
            return Type::vectorUInt4;
        default:
            break;
        }
    } else if (type.basetype == spirv_cross::SPIRType::BaseType::Double && type.columns == 1) {
        switch (type.vecsize) {
        case 1:
            return Type::scalarDouble;
        case 2:
            return Type::vectorDouble2;
        case 3:
            return Type::vectorDouble3;
        case 4:
            return Type::vectorDouble4;
        default:
            break;
        }
    }

    throw RendererError(fmt::format("Unknown type in shader {}: baseType={}, vecsize={}, columns={}", filename,
        magic_enum::enum_name(type.basetype), type.vecsize, type.columns));

    return Type::unknown;
}

} // namespace chronicle