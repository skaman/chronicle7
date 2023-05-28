// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "FileWatcherListener.h"

#include "InternalStorageContext.h"
#include "Storage/Common.h"

namespace chronicle::internal {

void FileWatcherListener::handleFileAction(efsw::WatchID watchid, const std::string& dir, const std::string& filename,
    efsw::Action action, std::string oldFilename)
{
    FileChangeEvent changeEvent {};

    const auto fullPath = (std::filesystem::path(dir) / std::filesystem::path(filename)).make_preferred();
    changeEvent.filename = _containerName.string() + fullPath.string().substr(_filesystemPath.string().length());

#ifdef CHRPLATFORM_WINDOWS
    std::ranges::replace(changeEvent.filename.begin(), changeEvent.filename.end(), '\\', '/');
#endif

    switch (action) {
    case efsw::Actions::Add:
        changeEvent.action = FileChangeAction::added;
        break;
    case efsw::Actions::Delete:
        changeEvent.action = FileChangeAction::deleted;
        break;
    case efsw::Actions::Modified:
        changeEvent.action = FileChangeAction::modified;
        break;
    case efsw::Actions::Moved:
        changeEvent.action = FileChangeAction::moved;
        break;
    default:
        return;
    }

    std::scoped_lock<std::mutex> lock(internal::InternalStorageContext::dispatcherMutex);
    InternalStorageContext::dispatcher.enqueue(changeEvent);
}

} // namespace chronicle::internal