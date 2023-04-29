// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "VulkanMesh.h"

#include "VulkanEnums.h"
#include "VulkanIndexBuffer.h"
#include "VulkanVertexBuffer.h"

namespace chronicle {

CHR_CONCRETE(VulkanMesh);

VulkanMesh::VulkanMesh(const std::vector<Submesh>& submeshes)
    : _submeshes(submeshes)
{
    // prepare the vector with the binding informations
    for (const auto& submesh : _submeshes) {
        VulkanMeshBindingInfo bindingInfo = {};
        bindingInfo.vertexBuffers.resize(submesh.vertexBuffers.size());
        bindingInfo.vertexBuffersOffsets.resize(submesh.vertexBuffers.size());
        for (auto i = 0; i < submesh.vertexBuffers.size(); i++) {
            const auto vertexBuffer = static_cast<VulkanVertexBuffer*>(submesh.vertexBuffers[i].get());
            bindingInfo.vertexBuffers[i] = vertexBuffer->buffer();
            bindingInfo.vertexBuffersOffsets[i] = 0;
        }

        const auto indexBuffer = static_cast<VulkanIndexBuffer*>(submesh.indexBuffer.get());
        bindingInfo.indexBuffer = indexBuffer->buffer();
        bindingInfo.indexBufferOffset = 0;
        bindingInfo.indexType = VulkanEnums::indexTypeToVulkan(submesh.indexType);
        _bindingInfos.push_back(std::move(bindingInfo));
    }
}

MeshRef VulkanMesh::create(const std::vector<Submesh>& submeshes)
{
    return std::make_shared<ConcreteVulkanMesh>(submeshes);
}

} // namespace chronicle