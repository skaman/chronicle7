// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

namespace chronicle::internal {

class FileWatcherListener : public efsw::FileWatchListener {
public:
    explicit FileWatcherListener(
        const std::filesystem::path& containerName, const std::filesystem::path& filesystemPath)
        : _containerName(containerName)
        , _filesystemPath(filesystemPath)
    {
    }

    void handleFileAction(efsw::WatchID watchid, const std::string& dir, const std::string& filename,
        efsw::Action action, std::string oldFilename) override;

private:
    const std::filesystem::path _containerName {};
    const std::filesystem::path _filesystemPath {};
};

} // namespace chronicle