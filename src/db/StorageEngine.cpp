#include "db/StorageEngine.hpp"
#include <string_view>

namespace db {

void StorageEngine::create_database(std::string_view name) {
    databases_.emplace(std::string(name), Database(std::string(name)));
}

void StorageEngine::drop_database(std::string_view name) {
    databases_.erase(std::string(name));
}

Database* StorageEngine::get_database(std::string_view name) noexcept {
    auto it = databases_.find(std::string(name));
    return it != databases_.end() ? &it->second : nullptr;
}

const Database* StorageEngine::get_database(std::string_view name) const noexcept {
    auto it = databases_.find(std::string(name));
    return it != databases_.end() ? &it->second : nullptr;
}

void to_json(json& j, const StorageEngine& engine) {
    j = json::object();
    j["databases"] = engine.databases_;
}

void from_json(const json& j, StorageEngine& engine) {
    engine.databases_.clear();
    j.at("databases").get_to(engine.databases_);
}

}
