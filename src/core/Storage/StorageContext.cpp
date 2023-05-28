// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "StorageContext.h"

namespace chronicle {

CHR_CONCRETE(File);

constexpr const char* ContentFolder = "Content";
constexpr const char* BuiltInFolder = "Built-In";

void StorageContext::init()
{
    CHRZONE_STORAGE;

    internal::InternalStorageContext::fileWatcher.watch();

    std::filesystem::path currentPath { std::filesystem::current_path() };

    registerPath(ContentFolder, (currentPath /= ContentFolder).make_preferred());

    // TODO: need a better solution
    std::filesystem::path debugPath { RESOURCES_DEBUG_PATH };
    registerPath(BuiltInFolder, (debugPath /= "Resources").make_preferred());
}

void StorageContext::deinit()
{
    CHRZONE_STORAGE;

    internal::InternalStorageContext::fileWatcherListeners.clear();
}

void StorageContext::poll()
{
    CHRZONE_STORAGE;
    std::scoped_lock<std::mutex> lock(internal::InternalStorageContext::dispatcherMutex);
    internal::InternalStorageContext::dispatcher.update();
}

bool StorageContext::exists(const std::string& filename)
{
    CHRZONE_STORAGE;

    try {
        return std::filesystem::exists(resolveFilename(filename));
    } catch (const StorageError&) {
        return false;
    }
}

File StorageContext::file(const std::string& filename)
{
    CHRZONE_STORAGE;

    return ConcreteFile(resolveFilename(filename));
}

void StorageContext::registerPath(
    const std::filesystem::path& containerName, const std::filesystem::path& filesystemPath)
{
    CHRZONE_STORAGE;

    internal::InternalStorageContext::pathMap[containerName] = filesystemPath;

    auto listener = std::make_unique<internal::FileWatcherListener>(containerName, filesystemPath);
    auto watchId
        = internal::InternalStorageContext::fileWatcher.addWatch(filesystemPath.string(), listener.get(), true);

    internal::InternalStorageContext ::fileWatcherListeners.insert(std::move(listener));
    internal::InternalStorageContext::fileWatcherIds.insert(watchId);
}

std::filesystem::path StorageContext::resolveFilename(const std::filesystem::path& filename)
{
    CHRZONE_STORAGE;

    if (filename.is_absolute()) {
        throw StorageError(fmt::format("Invalid path {}", filename.string()));
    }
    for (const auto& [prefix, path] : internal::InternalStorageContext::pathMap) {
        const auto prefixString = prefix.string();
        const auto filenameString = filename.string();
        if (filenameString.starts_with(prefixString)) {
            return path / filenameString.substr(prefixString.length() + 1);
        }
    }

    throw StorageError(fmt::format("Invalid path {}", filename.string()));
}

} // namespace chronicle