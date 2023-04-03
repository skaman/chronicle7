#include "VulkanImage.h"

namespace chronicle {

VulkanImage::VulkanImage(const vk::Device& device, const vk::Image& image, const vk::ImageView& imageView,
    int imageViewWidth, int imageViewHeight)
    : _device(device)
    , _image(image)
    , _imageView(imageView)
    , _width(imageViewWidth)
    , _height(imageViewHeight)
{
}

VulkanImage::~VulkanImage() { _device.destroyImageView(_imageView); }

void VulkanImage::updateImage(
    const vk::Image& image, const vk::ImageView& imageView, int imageViewWidth, int imageViewHeight)
{
    _device.destroyImageView(_imageView);

    _image = image;
    _imageView = imageView;
    _width = imageViewWidth;
    _height = imageViewHeight;

    updated();
}

} // namespace chronicle