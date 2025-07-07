#pragma once
#include "db/StorageEngine.hpp"

namespace db {
    bool save_to_file(const StorageEngine& engine, const std::string& path);
    bool load_from_file(StorageEngine& engine, const std::string& path);
}
