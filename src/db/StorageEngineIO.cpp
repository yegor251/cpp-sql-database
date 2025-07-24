#include "db/StorageEngineIO.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <string_view>
#include <string>

using json = nlohmann::json;
using namespace db;

// --- save/load ---

bool db::save_to_file(const StorageEngine& engine, std::string_view path) {
    std::ofstream ofs{std::string(path)};
    if (!ofs.is_open()) return false;
    json j = engine;
    ofs << j.dump(2);
    return true;
}

bool db::load_from_file(StorageEngine& engine, std::string_view path) {
    std::ifstream ifs{std::string(path)};
    if (!ifs.is_open()) return false;
    json j;
    ifs >> j;
    engine = j.get<StorageEngine>();
    return true;
}
