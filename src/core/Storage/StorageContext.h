// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "Common.h"
#include "File.h"
#include "Internal/InternalStorageContext.h"

namespace chronicle {

class StorageContext {
public:
    static void init();
    static void deinit();

    static void poll();

    [[nodiscard]] static bool exists(const std::string& filename);
    [[nodiscard]] static File file(const std::string& filename);

    template <typename Type> [[nodiscard]] static auto sink(const entt::id_type id = entt::type_hash<Type>::value())
    {
        return internal::InternalStorageContext::dispatcher.sink<Type>(id);
    }

private:
    static void registerPath(const std::filesystem::path& containerName, const std::filesystem::path& filesystemPath);
    [[nodiscard]] static std::filesystem::path resolveFilename(const std::filesystem::path& filename);
};

} // namespace chronicle