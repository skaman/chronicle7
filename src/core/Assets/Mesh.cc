// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "Mesh.h"

namespace chronicle {

CHR_CONCRETE(Mesh);

Mesh::Mesh(const std::vector<Submesh>& submeshes)
    : _submeshes(submeshes)
{
}

MeshRef Mesh::create(const std::vector<Submesh>& submeshes) { return std::make_shared<ConcreteMesh>(submeshes); }

} // namespace chronicle