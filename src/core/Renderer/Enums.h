// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

namespace chronicle {

/// @brief The stage shere the shader should be executed.
enum class ShaderStage {
    none = 0x00,
    fragment = 0x01,
    vertex = 0x02,
    compute = 0x04,

    _entt_enum_as_bitmask
};

/// @brief The type used by indices.
enum class IndexType {
    undefined,

    uint16,
    uint32
};

/// @brief Samples count for multi sampling anti aliasing
enum class MSAA { sampleCount1, sampleCount2, sampleCount4, sampleCount8, sampleCount16, sampleCount32, sampleCount64 };

/// @brief Data format for surface, texture or data structures.
enum class Format {
    undefined,

    // 8 bit signed int
    R8Sint,
    R8G8Sint,
    R8G8B8Sint,
    R8G8B8A8Sint,

    // 8 bit unsigned int
    R8Uint,
    R8G8Uint,
    R8G8B8Uint,
    R8G8B8A8Uint,

    // 8 bit normalized
    R8Unorm,
    R8G8Unorm,
    R8G8B8Unorm,
    R8G8B8A8Unorm,

    // 16 bit signed int
    R16Sint,
    R16G16Sint,
    R16G16B16Sint,
    R16G16B16A16Sint,

    // 16 bit unsigned int
    R16Uint,
    R16G16Uint,
    R16G16B16Uint,
    R16G16B16A16Uint,

    // 16 bit normalized
    R16Unorm,
    R16G16Unorm,
    R16G16B16Unorm,
    R16G16B16A16Unorm,

    // 32 bit signed int
    R32Sint,
    R32G32Sint,
    R32G32B32Sint,
    R32G32B32A32Sint,

    // 32 bit unsigned int
    R32Uint,
    R32G32Uint,
    R32G32B32Uint,
    R32G32B32A32Uint,

    // 32 bit signed float
    R32Sfloat,
    R32G32Sfloat,
    R32G32B32Sfloat,
    R32G32B32A32Sfloat,

    // TODO: other, to remove?
    B8G8R8A8Unorm,

    D32Sfloat,
    D32SfloatS8Uint,

    D24UnormS8Uint
};

/// @brief Specifies the type of a descriptor in a descriptor set.
enum class DescriptorType {
    unknown,
    sampler, ///< Specifies a sampler descriptor.
    combinedImageSampler, ///< Specifies a combined image sampler descriptor.
    sampledImage, ///< Specifies a sampled image descriptor.
    storageImage, ///< Specifies a storage image descriptor.
    uniformTexelBuffer, ///< Specifies a uniform texel buffer descriptor.
    storageTexelBuffer, ///< Specifies a storage texel buffer descriptor.
    uniformBuffer, ///< Specifies a uniform buffer descriptor.
    storageBuffer, ///< Specifies a storage buffer descriptor.
    uniformBufferDynamic, ///< Specifies a dynamic uniform buffer descriptor.
    storageBufferDynamic, ///< Specifies a dynamic storage buffer descriptor.
    inputAttachment, ///< Specifies an input attachment descriptor.
    accelerationStructure ///< Specifies an acceleration structure.
};

/// @brief Specified the type of a texture.
enum class TextureType {
    swapchain, ///< Swapchain texture.
    sampled, ///< Sampled texture.
    color, ///< Color texture.
    depth ///< Depth texture.
};

/// @brief Specify how contents of an attachment are treated at the beginning of a subpass.
enum class AttachmentLoadOp {
    load, ///< Specifies that the previous contents of the image within the render area will be preserved.
    clear, ///< Specifies that the contents within the render area will be cleared to a uniform value, which is
           ///< specified when a render pass instance is begun.
    dontCare ///< Specifies that the previous contents within the area need not be preserved; the contents of the
             ///< attachment will be undefined inside the render area.
};

/// @brief Specify how contents of an attachment are treated at the end of a subpass.
enum class AttachmentStoreOp {
    store, ///< Specifies the contents generated during the render pass and within the render area are written to
           ///< memory.
    dontCare ///< Specifies the contents within the render area are not needed after rendering, and may be discarded;
             ///< the contents of the attachment will be undefined inside the render area.
};

/// @brief Layout of image and image subresources.
enum class ImageLayout {
    undefined, ///< Specifies that the layout is unknown. Image memory cannot be transitioned into this layout.
    colorAttachment, ///< Must only be used as a color or resolve attachment in a Framebuffer.
    depthStencilAttachment, ///< Specifies a layout for both the depth and stencil aspects of a depth/stencil format
                            ///< image allowing read and write access as a depth/stencil attachment.
    shaderReadOnly, ///< Specifies a layout allowing read-only access in a shader as a sampled image, combined
                    ///< image/sampler, or input attachment.
    presentSrc ///< Must only be used for presenting a presentable image for display.
};

} // namespace chronicle