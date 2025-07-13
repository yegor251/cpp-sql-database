#pragma once
#include <unordered_map>
#include <string>
#include "db/Database.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace db {

class StorageEngine {
public:
    std::unordered_map<std::string, Database> databases;

    void create_database(const std::string& name);
    void drop_database(const std::string& name);
    Database* get_database(const std::string& name);

    friend void to_json(json& j, const StorageEngine& e);
    friend void from_json(const json& j, StorageEngine& e);
};

void to_json(json& j, const StorageEngine& engine);
void from_json(const json& j, StorageEngine& engine);

} // namespace db