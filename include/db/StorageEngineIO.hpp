#pragma once
#include "db/StorageEngine.hpp"
#include <string_view>

namespace db {
    bool save_to_file(const StorageEngine& engine, std::string_view path);
    bool load_from_file(StorageEngine& engine, std::string_view path);
}
