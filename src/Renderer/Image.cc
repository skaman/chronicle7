#include "Image.h"

#include "Renderer.h"

namespace chronicle {
Image::Image(const Renderer* renderer, const ImageInfo& imageInfo)
    : _image(renderer->native().device(), renderer->native().physicalDevice(), renderer->native().commandPool(),
        renderer->native().graphicsQueue(), imageInfo)
{
}

Image::Image(const vk::Device& device, const vk::Image& image, vk::Format format, int width, int height)
    : _image(device, image, format, width, height)
{
}

} // namespace chronicle