#pragma once
#include <unordered_map>
#include <string>
#include "db/Database.hpp"

namespace db {

class StorageEngine {
public:
    std::unordered_map<std::string, Database> databases;

    void create_database(const std::string& name) {
        databases.emplace(name, Database(name));
    }

    void drop_database(const std::string& name) {
        databases.erase(name);
    }

    Database* get_database(const std::string& name) {
        auto it = databases.find(name);
        if (it != databases.end()) return &it->second;
        return nullptr;
    }

    friend void to_json(json& j, const StorageEngine& e);
    friend void from_json(const json& j, StorageEngine& e);
};

inline void to_json(json& j, const StorageEngine& engine) {
    j = json::object();
    j["databases"] = json::object();
    for (const auto& [name, db] : engine.databases) {
        j["databases"][name] = db;
    }
}

inline void from_json(const json& j, StorageEngine& engine) {
    engine.databases.clear();
    for (auto it = j.at("databases").begin(); it != j.at("databases").end(); ++it) {
        engine.databases[it.key()] = it.value().get<Database>();
    }
}

} // namespace db