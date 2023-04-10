#pragma once

#include "pch.h"

#include "Renderer/Common.h"

namespace chronicle {

class MeshAsset;
using MeshAssetRef = std::shared_ptr<MeshAsset>;

class MeshAsset {
protected:
    explicit MeshAsset(const std::string& filename);

public:
    ~MeshAsset() = default;

    static MeshAssetRef load(const std::string& filename);

    [[nodiscard]] uint32_t shapesCount() const { return _shapesCount; }
    [[nodiscard]] const VertexBufferRef& vertexBuffer(uint32_t shapeIndex) const { return _vertexBuffers[shapeIndex]; }
    [[nodiscard]] const IndexBufferRef& indexBuffer(uint32_t shapeIndex) const { return _indexBuffers[shapeIndex]; }
    [[nodiscard]] uint32_t verticesCount(uint32_t shapeIndex) const { return _verticesCount[shapeIndex]; }
    [[nodiscard]] uint32_t indicesCount(uint32_t shapeIndex) const { return _indicesCount[shapeIndex]; }

private:
    std::vector<VertexBufferRef> _vertexBuffers;
    std::vector<IndexBufferRef> _indexBuffers;
    std::vector<uint32_t> _verticesCount;
    std::vector<uint32_t> _indicesCount;

    uint32_t _shapesCount;
};

} // namespace chronicle