// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "Gui.h"

#include "Common.h"
#include "Platform/GLFW/GLFWCommon.h"
#include "Renderer/Vulkan/VulkanCommandBuffer.h"
#include "Renderer/Vulkan/VulkanCommon.h"
#include "Renderer/Vulkan/VulkanRenderPass.h"
#include "Renderer/Vulkan/VulkanUtils.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

namespace chronicle {

static void check_vk_result(VkResult err)
{
    if (err == 0)
        return;
    spdlog::error("[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

void Gui::init(uint32_t swapChainImagesCount, const RenderPassRef& renderPass)
{
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

    GuiContex::descriptorPool = VulkanContext::device.createDescriptorPool(poolInfo, nullptr);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
    //  io.ConfigViewportsNoAutoMerge = true;
    //  io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    auto vulkanRenderPass = static_cast<VulkanRenderPass*>(renderPass.get());

    ImGui_ImplGlfw_InitForVulkan(GLFWContext::window, true);

    // Setup Platform/Renderer backends
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = VulkanContext::instance;
    init_info.PhysicalDevice = VulkanContext::physicalDevice;
    init_info.Device = VulkanContext::device;
    init_info.QueueFamily = VulkanContext::graphicsFamily;
    init_info.Queue = VulkanContext::graphicsQueue;
    init_info.PipelineCache = GuiContex::pipelineCache;
    init_info.DescriptorPool = GuiContex::descriptorPool;
    init_info.Subpass = 0;
    init_info.MinImageCount = 2;
    init_info.ImageCount = swapChainImagesCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = nullptr;
    init_info.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_Init(&init_info, vulkanRenderPass->renderPass());

    auto commandBuffer = VulkanUtils::beginSingleTimeCommands();
    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    VulkanUtils::endSingleTimeCommands(commandBuffer);
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void Gui::deinit()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    VulkanContext::device.destroyDescriptorPool(GuiContex::descriptorPool);
}

void Gui::newFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Gui::render(const CommandBufferRef& commandBuffer)
{
    auto vulkanCommandBuffer = static_cast<VulkanCommandBuffer*>(commandBuffer.get());

    ImGui::Render();

    ImDrawData* main_draw_data = ImGui::GetDrawData();
    // const bool main_is_minimized = (main_draw_data->DisplaySize.x <= 0.0f || main_draw_data->DisplaySize.y <= 0.0f);
    // if (!main_is_minimized)
    ImGui_ImplVulkan_RenderDrawData(main_draw_data, vulkanCommandBuffer->commandBuffer());

    // Update and Render additional Platform Windows
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

} // namespace chronicle
