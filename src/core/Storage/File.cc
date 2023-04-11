#include "File.h"

namespace chronicle {

std::vector<char> File::readBytes(const std::string& filename)
{
    CHRZONE_STORAGE

    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
        throw FileError(fmt::format("Failed to open file {}", filename));

    size_t fileSize = file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

} // namespace chronicle