#include "db/Database.hpp"

namespace db {

void Database::create_table(const std::string& table_name, const std::vector<std::string>& columns, const std::vector<std::string>& types) {
    tables.emplace(table_name, Table(table_name, columns, types));
}

void Database::drop_table(const std::string& table_name) {
    tables.erase(table_name);
}

void to_json(json& j, const Database& d) {
    j = json::object();
    j["name"] = d.name;
    j["tables"] = json::object();
    for (const auto& [name, table] : d.tables) {
        j["tables"][name] = table;
    }
}

void from_json(const json& j, Database& d) {
    j.at("name").get_to(d.name);
    d.tables.clear();
    for (auto it = j.at("tables").begin(); it != j.at("tables").end(); ++it) {
        Table table;
        it.value().get_to(table);
        d.tables[it.key()] = table;
    }
}

} // namespace db
