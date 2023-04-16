// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanImGui.h"

#include "VulkanCommandBuffer.h"
#include "VulkanCommon.h"
#include "VulkanUtils.h"

#ifdef GLFW_PLATFORM
#include "Platform/GLFW/GLFWCommon.h"
#include <backends/imgui_impl_glfw.h>
#endif

#include <backends/imgui_impl_vulkan.h>

namespace chronicle {

static void checkVulkanResult(VkResult err)
{
    if (err == 0)
        return;
    throw RendererError(fmt::format("Error: VkResult = {}", err));
}

void VulkanImGui::init()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("ImGui init");

    // create descriptor pool
    std::vector<vk::DescriptorPoolSize> sizes = { { vk::DescriptorType::eSampler, 1000 },
        { vk::DescriptorType::eCombinedImageSampler, 1000 }, { vk::DescriptorType::eSampledImage, 1000 },
        { vk::DescriptorType::eStorageImage, 1000 }, { vk::DescriptorType::eUniformTexelBuffer, 1000 },
        { vk::DescriptorType::eStorageTexelBuffer, 1000 }, { vk::DescriptorType::eUniformBuffer, 1000 },
        { vk::DescriptorType::eStorageBuffer, 1000 }, { vk::DescriptorType::eUniformBufferDynamic, 1000 },
        { vk::DescriptorType::eStorageBufferDynamic, 1000 }, { vk::DescriptorType::eInputAttachment, 1000 } };

    vk::DescriptorPoolCreateInfo poolInfo = {};
    poolInfo.setMaxSets(1000 * sizes.size());
    poolInfo.setPoolSizes(sizes);

    VulkanImGuiContext::descriptorPool = VulkanContext::device.createDescriptorPool(poolInfo, nullptr);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplGlfw_InitForVulkan(GLFWContext::window, true);

    // Setup Platform/Renderer backends
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = VulkanContext::instance;
    init_info.PhysicalDevice = VulkanContext::physicalDevice;
    init_info.Device = VulkanContext::device;
    init_info.QueueFamily = VulkanContext::graphicsFamily;
    init_info.Queue = VulkanContext::graphicsQueue;
    init_info.PipelineCache = VulkanImGuiContext::pipelineCache;
    init_info.DescriptorPool = VulkanImGuiContext::descriptorPool;
    init_info.Subpass = 0;
    init_info.MinImageCount = VulkanContext::maxFramesInFlight;
    init_info.ImageCount = static_cast<uint32_t>(VulkanContext::swapChainImages.size());
    //init_info.MSAASamples = static_cast<VkSampleCountFlagBits>(VulkanContext::msaaSamples);
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = nullptr;
    init_info.CheckVkResultFn = checkVulkanResult;
    ImGui_ImplVulkan_Init(&init_info, VulkanContext::debugRenderPass);

    auto commandBuffer = VulkanUtils::beginSingleTimeCommands();
    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    VulkanUtils::endSingleTimeCommands(commandBuffer);
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void VulkanImGui::deinit()
{
    CHRZONE_RENDERER;

    CHRLOG_DEBUG("ImGui deinit");

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    VulkanContext::device.destroyDescriptorPool(VulkanImGuiContext::descriptorPool);
}

void VulkanImGui::newFrame()

{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("ImGui new frame");

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void VulkanImGui::render(const CommandBufferRef& commandBuffer)
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("ImGui render");

    auto vulkanCommandBuffer = static_cast<VulkanCommandBuffer*>(commandBuffer.get());

    ImGui::Render();

    ImDrawData* main_draw_data = ImGui::GetDrawData();
    ImGui_ImplVulkan_RenderDrawData(main_draw_data, vulkanCommandBuffer->commandBuffer());

    // Update and Render additional Platform Windows
    const ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

} // namespace chronicle