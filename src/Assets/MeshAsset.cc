#include "MeshAsset.h"

#include "Locator.h"
#include "Renderer/Renderer.h"

namespace chronicle {

CHR_CONCRETE(MeshAsset);

MeshAsset::MeshAsset(const std::string& filename)
{
    //_vertexBuffer = Locator::renderer->createVertexBuffer();
    //_indexBuffer = Locator::renderer->createIndexBuffer();

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str())) {
        throw std::runtime_error(warn + err);
    }

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    _vertexBuffers.reserve(shapes.size());
    _indexBuffers.reserve(shapes.size());
    _verticesCount.reserve(shapes.size());
    _indicesCount.reserve(shapes.size());

    for (uint32_t shapeIndex = 0; shapeIndex < shapes.size(); shapeIndex++) {
        _vertexBuffers.push_back(Locator::renderer->createVertexBuffer());
        _indexBuffers.push_back(Locator::renderer->createIndexBuffer());

        const auto vertexBuffer = _vertexBuffers[shapeIndex].get();
        const auto indexBuffer = _indexBuffers[shapeIndex].get();

        for (const auto& index : shapes[shapeIndex].mesh.indices) {
            Vertex vertex {};

            vertex.pos = { attrib.vertices[3 * index.vertex_index + 0], attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2] };

            vertex.texCoord = { attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1] };

            vertex.color = { 1.0f, 1.0f, 1.0f };

            vertices.push_back(vertex);
            indices.push_back(static_cast<uint32_t>(indices.size()));
        }

        _verticesCount.push_back(static_cast<uint32_t>(vertices.size()));
        _indicesCount.push_back(static_cast<uint32_t>(indices.size()));

        vertexBuffer->set((void*)vertices.data(), sizeof(vertices[0]) * vertices.size());
        indexBuffer->set((void*)indices.data(), sizeof(indices[0]) * indices.size());
    }
}

MeshAssetRef MeshAsset::load(const std::string& filename) { return std::make_shared<ConcreteMeshAsset>(filename); }

} // namespace chronicle