#include "Image.h"

namespace chronicle {

Image::Image(const vk::Device& device, const vk::Image& image, const vk::ImageView& imageView, int imageViewWidth,
    int imageViewHeight)
    : _image(device, image, imageView, imageViewWidth, imageViewHeight)
{
}

} // namespace chronicle