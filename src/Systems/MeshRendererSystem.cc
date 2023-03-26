#include "MeshRendererSystem.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include <tiny_gltf.h>

#include <entt/entt.hpp>
#include <Renderer/Renderer.h>

namespace chronicle {

MeshRendererSystem::MeshRendererSystem()
{
    std::string filename = "D:\\Progetti\\glTF-Sample-Models\\2.0\\Triangle\\glTF-Embedded\\Triangle.gltf";
    // std::string filename = "D:\\Progetti\\glTF-Sample-Models\\2.0\\Sponza\\glTF\\Sponza.gltf";
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string err;
    std::string warn;
    auto test = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
}

void MeshRendererSystem::Run(entt::registry& registry)
{
    auto& renderer = entt::locator<Renderer>::value();

    renderer.DrawFrame();

    // https://github.com/SaschaWillems/Vulkan-glTF-PBR/blob/master/base/VulkanglTFModel.cpp
}

} // namespace chronicle