// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanImGui.h"

#include "Platform/Platform.h"
#include "Storage/Storage.h"
#include "VulkanCommandBuffer.h"
#include "VulkanCommon.h"
#include "VulkanRenderPass.h"
#include "VulkanUtils.h"

#ifdef GLFW_PLATFORM
#include "Platform/GLFW/GLFWCommon.h"
#include <backends/imgui_impl_glfw.h>
#endif

#include <backends/imgui_impl_vulkan.h>

namespace chronicle {

/// @brief ImGui callback for check the vulkan result
/// @param err Vulkan result
static void checkVulkanResult(VkResult err)
{
    if (err == 0)
        return;
    throw RendererError(fmt::format("Error: VkResult = {}", err));
}

void VulkanImGui::init()
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("ImGui init");

    // create descriptor pool
    std::vector<vk::DescriptorPoolSize> sizes = { { vk::DescriptorType::eSampler, 1000 },
        { vk::DescriptorType::eCombinedImageSampler, 1000 }, { vk::DescriptorType::eSampledImage, 1000 },
        { vk::DescriptorType::eStorageImage, 1000 }, { vk::DescriptorType::eUniformTexelBuffer, 1000 },
        { vk::DescriptorType::eStorageTexelBuffer, 1000 }, { vk::DescriptorType::eUniformBuffer, 1000 },
        { vk::DescriptorType::eStorageBuffer, 1000 }, { vk::DescriptorType::eUniformBufferDynamic, 1000 },
        { vk::DescriptorType::eStorageBufferDynamic, 1000 }, { vk::DescriptorType::eInputAttachment, 1000 } };
    vk::DescriptorPoolCreateInfo poolInfo = {};
    poolInfo.setMaxSets(static_cast<uint32_t>(1000 * sizes.size()));
    poolInfo.setPoolSizes(sizes);
    VulkanImGuiContext::descriptorPool = VulkanContext::device.createDescriptorPool(poolInfo, nullptr);

    // setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // get the scaling
    float scaling = Platform::windowDpiScale();

    // setup ImGui style
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
    style.ScaleAllSizes(scaling);

    // setup the font
    auto fontData = Storage::readBytes(":/bfont.ttf");
    ImFontConfig font_cfg;
    font_cfg.SizePixels = 11.0f * scaling;
    io.Fonts->AddFontFromMemoryTTF((void*)(fontData.data()), static_cast<int>(fontData.size()), 11.0f * scaling);

    // initialize glfw backend for vulkan
    ImGui_ImplGlfw_InitForVulkan(GLFWContext::window, true);

    // initialize vulkan backend
    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance = VulkanContext::instance;
    initInfo.PhysicalDevice = VulkanContext::physicalDevice;
    initInfo.Device = VulkanContext::device;
    initInfo.QueueFamily = VulkanContext::graphicsFamily;
    initInfo.Queue = VulkanContext::graphicsQueue;
    initInfo.PipelineCache = VulkanImGuiContext::pipelineCache;
    initInfo.DescriptorPool = VulkanImGuiContext::descriptorPool;
    initInfo.Subpass = 0;
    initInfo.MinImageCount = VulkanContext::maxFramesInFlight;
    initInfo.ImageCount = static_cast<uint32_t>(VulkanContext::imagesData.size());
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    initInfo.Allocator = nullptr;
    initInfo.CheckVkResultFn = checkVulkanResult;

    auto renderPass = static_cast<const vk::RenderPass*>(VulkanContext::debugRenderPass->renderPassId());
    ImGui_ImplVulkan_Init(&initInfo, *renderPass);

    // create font texture
    auto commandBuffer = VulkanUtils::beginSingleTimeCommands();
    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    VulkanUtils::endSingleTimeCommands(commandBuffer);
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void VulkanImGui::deinit()
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("ImGui deinit");

    // deinitialize vulkan backend
    ImGui_ImplVulkan_Shutdown();

    // deinitialize GLFW backend
    ImGui_ImplGlfw_Shutdown();

    // destroy the descriptor pool
    VulkanContext::device.destroyDescriptorPool(VulkanImGuiContext::descriptorPool);
}

void VulkanImGui::newFrame()

{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("ImGui new frame");

    // new frame for vulkan bakend
    ImGui_ImplVulkan_NewFrame();

    // new frame for GLFW backend
    ImGui_ImplGlfw_NewFrame();

    // imgui new frame
    ImGui::NewFrame();
}

void VulkanImGui::draw(const CommandBufferRef& commandBuffer)
{
    CHRZONE_RENDERER;

    CHRLOG_TRACE("ImGui render");

    // cast command buffer into vulkan command buffer
    auto vulkanCommandBuffer = static_cast<VulkanCommandBuffer*>(commandBuffer.get());

    // render imgui
    ImGui::Render();

    // draw imgui data
    ImDrawData* main_draw_data = ImGui::GetDrawData();
    ImGui_ImplVulkan_RenderDrawData(main_draw_data, vulkanCommandBuffer->commandBuffer());

    // update and render additional platform windows
    const ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

} // namespace chronicle