#include "db/Database.hpp"

namespace db {

Database::Database(std::string name) : name_(std::move(name)) {}

void Database::create_table(std::string_view table_name, const std::vector<std::string>& columns, const std::vector<std::string>& types, const std::vector<ForeignKey>& foreign_keys) {
    tables_.emplace(std::string(table_name), Table(std::string(table_name), columns, types, foreign_keys));
}

void Database::drop_table(std::string_view table_name) {
    tables_.erase(std::string(table_name));
}

const Table* Database::get_table(std::string_view table_name) const noexcept {
    auto it = tables_.find(std::string(table_name));
    return it != tables_.end() ? &it->second : nullptr;
}

Table* Database::get_table(std::string_view table_name) noexcept {
    auto it = tables_.find(std::string(table_name));
    return it != tables_.end() ? &it->second : nullptr;
}

void to_json(json& j, const Database& d) {
    j = json::object();
    j["name"] = d.name_;
    j["tables"] = d.tables_;
}

void from_json(const json& j, Database& d) {
    j.at("name").get_to(d.name_);
    j.at("tables").get_to(d.tables_);
}

}
