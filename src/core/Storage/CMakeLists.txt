add_subdirectory(Internal)

target_sources(chronicle-core
PRIVATE
    "Common.h"
    "File.cpp"
    "File.h"
    "StorageContext.cpp"
    "StorageContext.h"
)