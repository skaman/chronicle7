#include "Semaphore.h"

#include "Renderer.h"

namespace chronicle {

Semaphore::Semaphore(const Renderer* renderer)
    : _semaphore(renderer->native().device())
{
}

} // namespace chronicle