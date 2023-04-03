#include "Fence.h"

#include "Renderer.h"

namespace chronicle {

Fence::Fence(const Renderer* renderer)
    : _fence(renderer->native().device())
{
}

} // namespace chronicle