// Copyright (c) 2023 Sandro Cavazzoni
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include "pch.h"

namespace chronicle {

class StorageError : public std::runtime_error {
public:
    using runtime_error::runtime_error;

    explicit StorageError(const std::string& message)
        : runtime_error(message.c_str())
    {
    }
};

enum class FileChangeAction { added, deleted, modified, moved };

struct FileChangeEvent {
    std::string filename {};
    FileChangeAction action {};
};

class FileWatcher;
using FileWatcherUnique = std::unique_ptr<FileWatcher>;

} // namespace chronicle