#include "Renderer.h"

#include <spdlog/spdlog.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>
#include <fstream>
#include <iostream>
#include <set>

#include "Platform/App.h"

namespace chronicle {

const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> ValidationLayers = { "VK_LAYER_KHRONOS_validation" };

const std::vector<const char*> DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

#ifdef NDEBUG
const bool EnableValidationLayers = false;
#else
const bool EnableValidationLayers = true;
#endif

const std::vector<Vertex> Vertices
    = { { { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } }, { { 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
          { { 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } }, { { -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f } } };

const std::vector<uint16_t> Indices = { 0, 1, 2, 2, 3, 0 };

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    return func != nullptr ? func(instance, pCreateInfo, pAllocator, pCallback) : VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(
    VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
        func(instance, callback, pAllocator);
}

Renderer::Renderer(App* app)
    : _app(app)
{
    CreateInstance();
    SetupDebugCallback();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapChain();
    CreateImageViews();
    CreateRenderPass();
    CreateDescriptorSetLayout();
    CreateGraphicsPipeline();
    CreateFramebuffers();
    CreateCommandPool();
    CreateVertexBuffer();
    CreateIndexBuffer();
    CreateUniformBuffers();
    CreateDescriptorPool();
    CreateDescriptorSets();
    CreateCommandBuffers();
    CreateSyncObjects();
}

Renderer::~Renderer()
{
    CleanupSwapChain();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        _device.destroyBuffer(_uniformBuffers[i]);
        _device.freeMemory(_uniformBuffersMemory[i]);
    }

    _device.destroyDescriptorPool(_descriptorPool);
    _device.destroyDescriptorSetLayout(_descriptorSetLayout);

    _device.destroyPipeline(_graphicsPipeline);
    _device.destroyPipelineLayout(_pipelineLayout);
    _device.destroyRenderPass(_renderPass);

    _device.destroyBuffer(_vertexBuffer);
    _device.freeMemory(_vertexBufferMemory);

    _device.destroyBuffer(_indexBuffer);
    _device.freeMemory(_indexBufferMemory);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        _device.destroySemaphore(_renderFinishedSemaphores[i]);
        _device.destroySemaphore(_imageAvailableSemaphores[i]);
        _device.destroyFence(_inFlightFences[i]);
    }

    _device.destroyCommandPool(_commandPool);

    _device.destroy();

    if (EnableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(_instance, _debugCallback, nullptr);
    }

    _instance.destroySurfaceKHR(_surface);
    _instance.destroy();
}

void Renderer::CleanupSwapChain()
{
    for (const auto& framebuffer : _swapChainFramebuffers)
        _device.destroyFramebuffer(framebuffer);

    for (const auto& imageView : _swapChainImageViews)
        _device.destroyImageView(imageView);

    _device.destroySwapchainKHR(_swapChain);
}

void Renderer::RecreateSwapChain()
{
    int width = 0;
    int height = 0;
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(_app->Window(), &width, &height);
        glfwWaitEvents();
    }

    _device.waitIdle();

    CleanupSwapChain();

    CreateSwapChain();
    CreateImageViews();
    CreateFramebuffers();
}

void Renderer::CreateInstance()
{
    if (EnableValidationLayers && !CheckValidationLayerSupport())
        throw RendererError("validation layers requested, but not available!");

    auto appName = _app->AppName();
    auto const& appInfo = vk::ApplicationInfo()
                              .setPApplicationName(appName.c_str())
                              .setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
                              .setPEngineName("Chronicle")
                              .setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
                              .setApiVersion(VK_API_VERSION_1_0);

    auto extensions = GetRequiredExtensions();

    auto& createInfo = vk::InstanceCreateInfo().setPApplicationInfo(&appInfo).setPEnabledExtensionNames(extensions);

    auto debugCreateInfo = vk::DebugUtilsMessengerCreateInfoEXT();
    if (EnableValidationLayers) {
        PopulateDebugMessengerCreateInfo(debugCreateInfo);

        createInfo.setPEnabledLayerNames(ValidationLayers).setPNext(&debugCreateInfo);
    }

    _instance = vk::createInstance(createInfo, nullptr);
}

void Renderer::PopulateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo) const
{
    createInfo
        .setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
            | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
            | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
        .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
            | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
        .setPfnUserCallback(DebugCallback);
}

void Renderer::SetupDebugCallback()
{
    if (!EnableValidationLayers)
        return;

    auto createInfo = vk::DebugUtilsMessengerCreateInfoEXT();
    PopulateDebugMessengerCreateInfo(createInfo);

    // NOTE: Vulkan-hpp has methods for this, but they trigger linking errors...
    // instance->createDebugUtilsMessengerEXT(createInfo);
    // instance->createDebugUtilsMessengerEXTUnique(createInfo);

    // NOTE: reinterpret_cast is also used by vulkan.hpp internally for all these structs
    if (CreateDebugUtilsMessengerEXT(_instance,
            reinterpret_cast<const VkDebugUtilsMessengerCreateInfoEXT*>(&createInfo), nullptr, &_debugCallback)
        != VK_SUCCESS)
        throw RendererError("failed to set up debug callback!");
}

void Renderer::CreateSurface()
{
    VkSurfaceKHR rawSurface;
    if (glfwCreateWindowSurface(_instance, _app->Window(), nullptr, &rawSurface) != VK_SUCCESS)
        throw RendererError("failed to create window surface!");

    _surface = rawSurface;
}

void Renderer::PickPhysicalDevice()
{
    auto devices = _instance.enumeratePhysicalDevices();
    if (devices.size() == 0)
        throw RendererError("failed to find GPUs with Vulkan support!");

    for (const auto& device : devices) {
        if (IsDeviceSuitable(device)) {
            _physicalDevice = device;
            break;
        }
    }

    if (!_physicalDevice)
        throw RendererError("failed to find a suitable GPU!");
}

void Renderer::CreateLogicalDevice()
{
    QueueFamilyIndices indices = FindQueueFamilies(_physicalDevice);

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };
    std::array<float, 1> queuePriorities = { 1.0f };

    for (auto queueFamily : uniqueQueueFamilies) {
        queueCreateInfos.push_back(vk::DeviceQueueCreateInfo()
                                       .setQueueFamilyIndex(queueFamily)
                                       .setQueueCount(1)
                                       .setQueuePriorities(queuePriorities));
    }

    auto deviceFeatures = vk::PhysicalDeviceFeatures();
    auto& createInfo = vk::DeviceCreateInfo()
                           .setQueueCreateInfos(queueCreateInfos)
                           .setPEnabledFeatures(&deviceFeatures)
                           .setPEnabledExtensionNames(DeviceExtensions);

    if (EnableValidationLayers)
        createInfo.setPEnabledLayerNames(ValidationLayers);

    _device = _physicalDevice.createDevice(createInfo);

    _graphicsQueue = _device.getQueue(indices.graphicsFamily.value(), 0);
    _presentQueue = _device.getQueue(indices.presentFamily.value(), 0);
}

void Renderer::CreateSwapChain()
{
    auto swapChainSupport = QuerySwapChainSupport(_physicalDevice);

    vk::SurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    vk::PresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    vk::Extent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        imageCount = swapChainSupport.capabilities.maxImageCount;

    auto indices = FindQueueFamilies(_physicalDevice);
    std::array<uint32_t, 2> queueFamilyIndices = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    auto& createInfo = vk::SwapchainCreateInfoKHR()
                           .setSurface(_surface)
                           .setMinImageCount(imageCount)
                           .setImageFormat(surfaceFormat.format)
                           .setImageColorSpace(surfaceFormat.colorSpace)
                           .setImageExtent(extent)
                           .setImageArrayLayers(1)
                           .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
                           .setPreTransform(swapChainSupport.capabilities.currentTransform)
                           .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
                           .setPresentMode(presentMode)
                           .setClipped(true);

    if (indices.graphicsFamily != indices.presentFamily)
        createInfo.setImageSharingMode(vk::SharingMode::eConcurrent).setQueueFamilyIndices(queueFamilyIndices);
    else
        createInfo.setImageSharingMode(vk::SharingMode::eExclusive);

    _swapChain = _device.createSwapchainKHR(createInfo);

    _swapChainImages = _device.getSwapchainImagesKHR(_swapChain);

    _swapChainImageFormat = surfaceFormat.format;
    _swapChainExtent = extent;
}

void Renderer::CreateImageViews()
{
    _swapChainImageViews.resize(_swapChainImages.size());

    for (size_t i = 0; i < _swapChainImages.size(); i++) {
        using enum vk::ComponentSwizzle;
        const auto& components = vk::ComponentMapping().setR(eIdentity).setG(eIdentity).setB(eIdentity).setA(eIdentity);

        const auto& subresourceRange = vk::ImageSubresourceRange()
                                           .setAspectMask(vk::ImageAspectFlagBits::eColor)
                                           .setBaseMipLevel(0)
                                           .setLevelCount(1)
                                           .setBaseArrayLayer(0)
                                           .setLayerCount(1);

        const auto& createInfo = vk::ImageViewCreateInfo()
                                     .setImage(_swapChainImages[i])
                                     .setViewType(vk::ImageViewType::e2D)
                                     .setFormat(_swapChainImageFormat)
                                     .setComponents(components)
                                     .setSubresourceRange(subresourceRange);

        _swapChainImageViews[i] = _device.createImageView(createInfo);
    }
}

void Renderer::CreateRenderPass()
{
    const auto& colorAttachment = vk::AttachmentDescription()
                                      .setFormat(_swapChainImageFormat)
                                      .setSamples(vk::SampleCountFlagBits::e1)
                                      .setLoadOp(vk::AttachmentLoadOp::eClear)
                                      .setStoreOp(vk::AttachmentStoreOp::eStore)
                                      .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                                      .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                                      .setInitialLayout(vk::ImageLayout::eUndefined)
                                      .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    const auto& colorAttachmentRef
        = vk::AttachmentReference().setAttachment(0).setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    const auto& subpass = vk::SubpassDescription()
                              .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                              .setColorAttachments(colorAttachmentRef);

    const auto& dependency = vk::SubpassDependency()
                                 .setSrcSubpass(VK_SUBPASS_EXTERNAL)
                                 .setDstSubpass(0)
                                 .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                                 .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                                 .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

    const auto& renderPassInfo
        = vk::RenderPassCreateInfo().setAttachments(colorAttachment).setSubpasses(subpass).setDependencies(dependency);

    _renderPass = _device.createRenderPass(renderPassInfo);
}

void Renderer::CreateDescriptorSetLayout()
{
    const auto& uboLayoutBinding = vk::DescriptorSetLayoutBinding()
                                       .setBinding(0)
                                       .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                                       .setDescriptorCount(1)
                                       .setStageFlags(vk::ShaderStageFlagBits::eVertex);

    const auto& layoutInfo = vk::DescriptorSetLayoutCreateInfo().setBindings(uboLayoutBinding);
    _descriptorSetLayout = _device.createDescriptorSetLayout(layoutInfo);
}

void Renderer::CreateGraphicsPipeline()
{
    auto vertShaderCode = ReadFile("Shaders/triangle.vert.bin");
    auto fragShaderCode = ReadFile("Shaders/triangle.frag.bin");

    auto vertShaderModule = CreateShaderModule(vertShaderCode);
    auto fragShaderModule = CreateShaderModule(fragShaderCode);

    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = { vk::PipelineShaderStageCreateInfo()
                                                                          .setStage(vk::ShaderStageFlagBits::eVertex)
                                                                          .setModule(*vertShaderModule)
                                                                          .setPName("main"),
        vk::PipelineShaderStageCreateInfo()
            .setStage(vk::ShaderStageFlagBits::eFragment)
            .setModule(*fragShaderModule)
            .setPName("main") };

    auto bindingDescription = Vertex::GetBindingDescription();
    auto attributeDescriptions = Vertex::GetAttributeDescriptions();

    const auto& vertexInputInfo = vk::PipelineVertexInputStateCreateInfo()
                                      .setVertexAttributeDescriptions(attributeDescriptions)
                                      .setVertexBindingDescriptions(bindingDescription);

    const auto& inputAssembly = vk::PipelineInputAssemblyStateCreateInfo()
                                    .setTopology(vk::PrimitiveTopology::eTriangleList)
                                    .setPrimitiveRestartEnable(false);

    const auto& viewportState = vk::PipelineViewportStateCreateInfo().setViewportCount(1).setScissorCount(1);

    const auto& rasterizer = vk::PipelineRasterizationStateCreateInfo()
                                 .setDepthClampEnable(false)
                                 .setRasterizerDiscardEnable(false)
                                 .setPolygonMode(vk::PolygonMode::eFill)
                                 .setLineWidth(1.0f)
                                 .setCullMode(vk::CullModeFlagBits::eBack)
                                 .setFrontFace(vk::FrontFace::eCounterClockwise)
                                 .setDepthBiasEnable(false);

    const auto& multisampling
        = vk::PipelineMultisampleStateCreateInfo().setSampleShadingEnable(false).setRasterizationSamples(
            vk::SampleCountFlagBits::e1);

    const auto& colorBlendAttachment
        = vk::PipelineColorBlendAttachmentState()
              .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
                  | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
              .setBlendEnable(false);

    const auto& colorBlending = vk::PipelineColorBlendStateCreateInfo()
                                    .setLogicOpEnable(false)
                                    .setLogicOp(vk::LogicOp::eCopy)
                                    .setAttachments(colorBlendAttachment)
                                    .setBlendConstants({ 0.0f, 0.0f, 0.0f, 0.0f });

    std::array<vk::DynamicState, 2> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };

    const auto& dynamicState = vk::PipelineDynamicStateCreateInfo().setDynamicStates(dynamicStates);

    const auto& pipelineLayoutInfo = vk::PipelineLayoutCreateInfo().setSetLayouts(_descriptorSetLayout);

    _pipelineLayout = _device.createPipelineLayout(pipelineLayoutInfo);

    const auto& pipelineInfo = vk::GraphicsPipelineCreateInfo()
                                   .setStages(shaderStages)
                                   .setPVertexInputState(&vertexInputInfo)
                                   .setPInputAssemblyState(&inputAssembly)
                                   .setPViewportState(&viewportState)
                                   .setPRasterizationState(&rasterizer)
                                   .setPMultisampleState(&multisampling)
                                   .setPColorBlendState(&colorBlending)
                                   .setPDynamicState(&dynamicState)
                                   .setLayout(_pipelineLayout)
                                   .setRenderPass(_renderPass)
                                   .setSubpass(0);

    vk::Result result;
    std::tie(result, _graphicsPipeline) = _device.createGraphicsPipeline(nullptr, pipelineInfo);
    if (result != vk::Result::eSuccess)
        throw RendererError("failed to create graphics pipeline!");
}

void Renderer::CreateFramebuffers()
{
    _swapChainFramebuffers.resize(_swapChainImageViews.size());

    for (size_t i = 0; i < _swapChainImageViews.size(); i++) {
        std::array<vk::ImageView, 1> attachments = { _swapChainImageViews[i] };

        const auto& framebufferInfo = vk::FramebufferCreateInfo()
                                          .setRenderPass(_renderPass)
                                          .setAttachments(attachments)
                                          .setWidth(_swapChainExtent.width)
                                          .setHeight(_swapChainExtent.height)
                                          .setLayers(1);

        _swapChainFramebuffers[i] = _device.createFramebuffer(framebufferInfo);
    }
}

void Renderer::CreateCommandPool()
{
    auto queueFamilyIndices = FindQueueFamilies(_physicalDevice);
    const auto& poolInfo = vk::CommandPoolCreateInfo()
                               .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
                               .setQueueFamilyIndex(queueFamilyIndices.graphicsFamily.value());

    _commandPool = _device.createCommandPool(poolInfo);
}

void Renderer::CreateVertexBuffer()
{
    vk::DeviceSize bufferSize = sizeof(Vertices[0]) * Vertices.size();

    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;
    CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer,
        stagingBufferMemory);

    void* data = _device.mapMemory(stagingBufferMemory, 0, bufferSize);
    memcpy(data, Vertices.data(), bufferSize);
    _device.unmapMemory(stagingBufferMemory);

    CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal, _vertexBuffer, _vertexBufferMemory);

    CopyBuffer(stagingBuffer, _vertexBuffer, bufferSize);

    _device.destroyBuffer(stagingBuffer);
    _device.freeMemory(stagingBufferMemory);
}

void Renderer::CreateIndexBuffer()
{
    vk::DeviceSize bufferSize = sizeof(Indices[0]) * Indices.size();

    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;
    CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer,
        stagingBufferMemory);

    void* data = _device.mapMemory(stagingBufferMemory, 0, bufferSize);
    memcpy(data, Indices.data(), bufferSize);
    _device.unmapMemory(stagingBufferMemory);

    CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal, _indexBuffer, _indexBufferMemory);

    CopyBuffer(stagingBuffer, _indexBuffer, bufferSize);

    _device.destroyBuffer(stagingBuffer);
    _device.freeMemory(stagingBufferMemory);
}

void Renderer::CreateUniformBuffers()
{
    vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

    _uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    _uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    _uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, _uniformBuffers[i],
            _uniformBuffersMemory[i]);

        _uniformBuffersMapped[i] = _device.mapMemory(_uniformBuffersMemory[i], 0, bufferSize);
    }
}

void Renderer::CreateDescriptorPool()
{
    const auto& poolSize = vk::DescriptorPoolSize().setDescriptorCount(MAX_FRAMES_IN_FLIGHT);
    const auto& poolInfo = vk::DescriptorPoolCreateInfo().setPoolSizes(poolSize).setMaxSets(MAX_FRAMES_IN_FLIGHT);

    _descriptorPool = _device.createDescriptorPool(poolInfo, nullptr);
}

void Renderer::CreateDescriptorSets()
{
    std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, _descriptorSetLayout);
    const auto& allocInfo = vk::DescriptorSetAllocateInfo().setDescriptorPool(_descriptorPool).setSetLayouts(layouts);

    _descriptorSets = _device.allocateDescriptorSets(allocInfo);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        const auto& bufferInfo = vk::DescriptorBufferInfo()
                                     .setBuffer(_uniformBuffers[i])
                                     .setOffset(0)
                                     .setRange(sizeof(UniformBufferObject));

        const auto& descriptorWrite = vk::WriteDescriptorSet()
                                          .setDstSet(_descriptorSets[i])
                                          .setDstBinding(0)
                                          .setDstArrayElement(0)
                                          .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                                          .setDescriptorCount(1)
                                          .setBufferInfo(bufferInfo);

        _device.updateDescriptorSets(descriptorWrite, 0);
    }
}

void Renderer::CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
    vk::Buffer& buffer, vk::DeviceMemory& bufferMemory)
{
    const auto& bufferInfo
        = vk::BufferCreateInfo().setSize(size).setUsage(usage).setSharingMode(vk::SharingMode::eExclusive);

    buffer = _device.createBuffer(bufferInfo);

    vk::MemoryRequirements memRequirements = _device.getBufferMemoryRequirements(buffer);

    const auto& allocInfo = vk::MemoryAllocateInfo()
                                .setAllocationSize(memRequirements.size)
                                .setMemoryTypeIndex(FindMemoryType(memRequirements.memoryTypeBits, properties));

    bufferMemory = _device.allocateMemory(allocInfo);

    _device.bindBufferMemory(buffer, bufferMemory, 0);
}

void Renderer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const
{
    const auto& allocInfo = vk::CommandBufferAllocateInfo()
                                .setLevel(vk::CommandBufferLevel::ePrimary)
                                .setCommandPool(_commandPool)
                                .setCommandBufferCount(1);

    auto commandBuffer = _device.allocateCommandBuffers(allocInfo)[0];
    const auto& beginInfo = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    const auto& copyRegion = vk::BufferCopy().setSize(size);

    commandBuffer.begin(beginInfo);
    commandBuffer.copyBuffer(srcBuffer, dstBuffer, copyRegion);
    commandBuffer.end();

    const auto& submitInfo = vk::SubmitInfo().setCommandBuffers(commandBuffer);

    _graphicsQueue.submit(submitInfo, nullptr);
    _graphicsQueue.waitIdle();

    _device.freeCommandBuffers(_commandPool, commandBuffer);
}

uint32_t Renderer::FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const
{
    auto memProperties = _physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw RendererError("failed to find suitable memory type!");
}

void Renderer::CreateCommandBuffers()
{
    _commandBuffers.resize(_swapChainFramebuffers.size());

    const auto& allocInfo = vk::CommandBufferAllocateInfo()
                                .setCommandPool(_commandPool)
                                .setLevel(vk::CommandBufferLevel::ePrimary)
                                .setCommandBufferCount(static_cast<uint32_t>(_commandBuffers.size()));

    _commandBuffers = _device.allocateCommandBuffers(allocInfo);
}

void Renderer::RecordCommandBuffer(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex)
{
    const auto& beginInfo = vk::CommandBufferBeginInfo();
    commandBuffer.begin(beginInfo);

    vk::ClearValue clearColor = { std::array<float, 4> { 0.0f, 0.0f, 0.0f, 1.0f } };
    const auto& renderPassInfo = vk::RenderPassBeginInfo()
                                     .setRenderPass(_renderPass)
                                     .setFramebuffer(_swapChainFramebuffers[imageIndex])
                                     .setRenderArea(vk::Rect2D({ 0, 0 }, _swapChainExtent))
                                     .setClearValues(clearColor);

    std::array<vk::Buffer, 1> vertexBuffers = { _vertexBuffer };
    std::array<vk::DeviceSize, 1> offsets = { 0 };

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _graphicsPipeline);

    const auto& viewport = vk::Viewport()
                               .setX(0.0f)
                               .setY(0.0f)
                               .setWidth(static_cast<float>(_swapChainExtent.width))
                               .setHeight(static_cast<float>(_swapChainExtent.height))
                               .setMinDepth(0.0f)
                               .setMaxDepth(1.0f);
    commandBuffer.setViewport(0, viewport);

    const auto& scissor = vk::Rect2D({ 0, 0 }, _swapChainExtent);
    commandBuffer.setScissor(0, scissor);

    commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);
    commandBuffer.bindIndexBuffer(_indexBuffer, 0, vk::IndexType::eUint16);
    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, _pipelineLayout, 0, _descriptorSets[_currentFrame], nullptr);
    commandBuffer.drawIndexed(static_cast<uint32_t>(Indices.size()), 1, 0, 0, 0);
    commandBuffer.endRenderPass();
    commandBuffer.end();
}

void Renderer::CreateSyncObjects()
{
    _imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        _imageAvailableSemaphores[i] = _device.createSemaphore({});
        _renderFinishedSemaphores[i] = _device.createSemaphore({});
        _inFlightFences[i] = _device.createFence({ vk::FenceCreateFlagBits::eSignaled });
    }
}

void Renderer::UpdateUniformBuffer(uint32_t currentImage)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo {};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f),
        static_cast<float>(_swapChainExtent.width) / static_cast<float>(_swapChainExtent.height), 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;
    memcpy(_uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void Renderer::WaitIdle() const { _device.waitIdle(); }

void Renderer::DrawFrame()
{
    (void)_device.waitForFences(_inFlightFences[_currentFrame], true, std::numeric_limits<uint64_t>::max());

    uint32_t imageIndex;
    try {
        auto result = _device.acquireNextImageKHR(
            _swapChain, std::numeric_limits<uint64_t>::max(), _imageAvailableSemaphores[_currentFrame], nullptr);
        imageIndex = result.value;
    } catch (vk::OutOfDateKHRError err) {
        RecreateSwapChain();
        return;
    }

    (void)_device.resetFences(_inFlightFences[_currentFrame]);
    _commandBuffers[_currentFrame].reset();

    RecordCommandBuffer(_commandBuffers[_currentFrame], imageIndex);
    UpdateUniformBuffer(_currentFrame);

    std::array<vk::Semaphore, 1> waitSemaphores = { _imageAvailableSemaphores[_currentFrame] };
    std::array<vk::PipelineStageFlags, 1> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    std::array<vk::Semaphore, 1> signalSemaphores = { _renderFinishedSemaphores[_currentFrame] };

    const auto& submitInfo = vk::SubmitInfo()
                                 .setWaitSemaphores(waitSemaphores)
                                 .setWaitDstStageMask(waitStages)
                                 .setCommandBuffers(_commandBuffers[_currentFrame])
                                 .setSignalSemaphores(signalSemaphores);

    _graphicsQueue.submit(submitInfo, _inFlightFences[_currentFrame]);

    std::array<vk::SwapchainKHR, 1> swapChains = { _swapChain };

    const auto& presentInfo = vk::PresentInfoKHR()
                                  .setWaitSemaphores(signalSemaphores)
                                  .setSwapchains(swapChains)
                                  .setImageIndices(imageIndex);

    vk::Result resultPresent;
    try {
        resultPresent = _presentQueue.presentKHR(presentInfo);
    } catch (vk::OutOfDateKHRError err) {
        resultPresent = vk::Result::eErrorOutOfDateKHR;
    }

    if (resultPresent == vk::Result::eSuboptimalKHR || resultPresent == vk::Result::eSuboptimalKHR
        || _framebufferResized) {
        _framebufferResized = false;
        RecreateSwapChain();
        return;
    }

    _currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

bool Renderer::CheckValidationLayerSupport() const
{
    auto availableLayers = vk::enumerateInstanceLayerProperties();
    for (const char* layerName : ValidationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
            return false;
    }

    return true;
}

std::vector<const char*> Renderer::GetRequiredExtensions() const
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (EnableValidationLayers)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensions;
}

bool Renderer::IsDeviceSuitable(const vk::PhysicalDevice& device) const
{
    QueueFamilyIndices indices = FindQueueFamilies(device);

    bool extensionsSupported = CheckDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return indices.IsComplete() && extensionsSupported && swapChainAdequate;
}

bool Renderer::CheckDeviceExtensionSupport(const vk::PhysicalDevice& device) const
{
    std::set<std::string, std::less<>> requiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

    for (const auto& extension : device.enumerateDeviceExtensionProperties())
        requiredExtensions.erase(extension.extensionName);

    return requiredExtensions.empty();
}

QueueFamilyIndices Renderer::FindQueueFamilies(vk::PhysicalDevice device) const
{
    QueueFamilyIndices indices;

    auto queueFamilies = device.getQueueFamilyProperties();

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
            indices.graphicsFamily = i;

        if (queueFamily.queueCount > 0 && device.getSurfaceSupportKHR(i, _surface))
            indices.presentFamily = i;

        if (indices.IsComplete())
            break;

        i++;
    }

    return indices;
}

SwapChainSupportDetails Renderer::QuerySwapChainSupport(const vk::PhysicalDevice& device) const
{
    SwapChainSupportDetails details;
    details.capabilities = device.getSurfaceCapabilitiesKHR(_surface);
    details.formats = device.getSurfaceFormatsKHR(_surface);
    details.presentModes = device.getSurfacePresentModesKHR(_surface);
    return details;
}

vk::SurfaceFormatKHR Renderer::ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) const
{
    if (availableFormats.size() == 1 && availableFormats[0].format == vk::Format::eUndefined)
        return { vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };

    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Unorm
            && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            return availableFormat;
    }

    return availableFormats[0];
}

vk::PresentModeKHR Renderer::ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) const
{
    vk::PresentModeKHR bestMode = vk::PresentModeKHR::eFifo;

    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox)
            return availablePresentMode;
        else if (availablePresentMode == vk::PresentModeKHR::eImmediate)
            bestMode = availablePresentMode;
    }

    return bestMode;
}

vk::Extent2D Renderer::ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width;
        int height;
        glfwGetFramebufferSize(_app->Window(), &width, &height);

        vk::Extent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

        actualExtent.width = std::max(
            capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(
            capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

vk::UniqueShaderModule Renderer::CreateShaderModule(const std::vector<char>& code) const
{
    return _device.createShaderModuleUnique(
        { vk::ShaderModuleCreateFlags(), code.size(), reinterpret_cast<const uint32_t*>(code.data()) });
}

std::vector<char> Renderer::ReadFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
        throw std::runtime_error("failed to open file!");

    size_t fileSize = file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

VKAPI_ATTR VkBool32 VKAPI_CALL Renderer::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        spdlog::error("{}", pCallbackData->pMessage);
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        spdlog::warn("{}", pCallbackData->pMessage);
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        spdlog::info("{}", pCallbackData->pMessage);
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        spdlog::debug("{}", pCallbackData->pMessage);
    return 0;
}

} // namespace chronicle