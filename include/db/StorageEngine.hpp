#pragma once
#include <unordered_map>
#include <string>
#include <string_view>
#include <optional>
#include "db/Database.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace db {

class StorageEngine {
public:
    StorageEngine() = default;

    void create_database(std::string_view name);
    void drop_database(std::string_view name);

    [[nodiscard]] Database* get_database(std::string_view name) noexcept;
    [[nodiscard]] const Database* get_database(std::string_view name) const noexcept;

    const std::unordered_map<std::string, Database>& get_databases() const noexcept { return databases_; }

    friend void to_json(json& j, const StorageEngine& e);
    friend void from_json(const json& j, StorageEngine& e);

private:
    std::unordered_map<std::string, Database> databases_;
};

void to_json(json& j, const StorageEngine& engine);
void from_json(const json& j, StorageEngine& engine);

} // namespace db