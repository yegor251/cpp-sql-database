#include "db/StorageEngine.hpp"

namespace db {

void StorageEngine::create_database(const std::string& name) {
    databases.emplace(name, Database(name));
}

void StorageEngine::drop_database(const std::string& name) {
    databases.erase(name);
}

Database* StorageEngine::get_database(const std::string& name) {
    auto it = databases.find(name);
    if (it != databases.end()) return &it->second;
    return nullptr;
}

void to_json(json& j, const StorageEngine& engine) {
    j = json::object();
    j["databases"] = json::object();
    for (const auto& [name, db] : engine.databases) {
        j["databases"][name] = db;
    }
}

void from_json(const json& j, StorageEngine& engine) {
    engine.databases.clear();
    for (auto it = j.at("databases").begin(); it != j.at("databases").end(); ++it) {
        engine.databases[it.key()] = it.value().get<Database>();
    }
}

} // namespace db
