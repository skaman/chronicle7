#include "DescriptorSet.h"

#include "Renderer.h"

namespace chronicle {

DescriptorSet::DescriptorSet(const Renderer* renderer)
    : _descriptorSet(renderer->native().device(), renderer->native().physicalDevice())
{
}

} // namespace chronicle