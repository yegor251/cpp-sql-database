#pragma once
#include <string>
#include <unordered_map>
#include "db/Table.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace db {

struct Database {
    std::string name;
    std::unordered_map<std::string, Table> tables;

    Database() = default;
    Database(const std::string& name) : name(name) {}

    void create_table(const std::string& table_name, const std::vector<std::string>& columns, const std::vector<std::string>& types) {
        tables.emplace(table_name, Table(table_name, columns, types));
    }

    void drop_table(const std::string& table_name) {
        tables.erase(table_name);
    }

    friend void to_json(json& j, const Database& d);
    friend void from_json(const json& j, Database& d);
};

inline void to_json(json& j, const Database& d) {
    j = json::object();
    j["name"] = d.name;
    j["tables"] = json::object();
    for (const auto& [name, table] : d.tables) {
        j["tables"][name] = table;
    }
}

inline void from_json(const json& j, Database& d) {
    j.at("name").get_to(d.name);
    d.tables.clear();
    for (auto it = j.at("tables").begin(); it != j.at("tables").end(); ++it) {
        Table table;
        it.value().get_to(table);
        d.tables[it.key()] = table;
    }
}

} // namespace db