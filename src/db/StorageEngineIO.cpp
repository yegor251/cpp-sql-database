#include "db/StorageEngineIO.hpp"
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace db;

// --- save/load ---

bool db::save_to_file(const StorageEngine& engine, const std::string& path) {
    std::ofstream ofs(path);
    if (!ofs) return false;
    json j = engine;
    ofs << j.dump(2);
    return true;
}

bool db::load_from_file(StorageEngine& engine, const std::string& path) {
    std::ifstream ifs(path);
    if (!ifs) return false;
    json j;
    ifs >> j;
    engine = j.get<StorageEngine>();
    return true;
}
