// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#define TINYGLTF_IMPLEMENTATION
#include <tiny_gltf.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

std::string chronicle::join(const std::vector<std::string>& data)
{
    const char* const delim = ", ";

    std::ostringstream imploded;
    std::ranges::copy(data.begin(), data.end(), std::ostream_iterator<std::string>(imploded, delim));
    return imploded.str();
}

//#ifndef NDEBUG
//
//void* operator new(std::size_t count)
//{
//    auto ptr = malloc(count);
//    TracyAlloc(ptr, count);
//    return ptr;
//}
//void operator delete(void* ptr) noexcept
//{
//    TracyFree(ptr);
//    free(ptr);
//}
//
// #endif
