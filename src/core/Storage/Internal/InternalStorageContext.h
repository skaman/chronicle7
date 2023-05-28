// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

#include "FileWatcherListener.h"

namespace chronicle::internal {

struct InternalStorageContext {
    static inline std::unordered_map<std::filesystem::path, std::filesystem::path> pathMap {};
    static inline efsw::FileWatcher fileWatcher {};
    static inline std::unordered_set<std::unique_ptr<internal::FileWatcherListener>> fileWatcherListeners {};
    static inline std::unordered_set<efsw::WatchID> fileWatcherIds {};
    static inline entt::dispatcher dispatcher {};
    static inline std::mutex dispatcherMutex {};
};

} // namespace chronicle::internal