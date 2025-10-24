#include "db/Table.hpp"

namespace db {

void to_json(json& j, const ForeignKey& fk) {
    j = json::object();
    j["column_name"] = fk.column_name;
    j["referenced_table"] = fk.referenced_table;
    j["referenced_column"] = fk.referenced_column;
}

void from_json(const json& j, ForeignKey& fk) {
    j.at("column_name").get_to(fk.column_name);
    j.at("referenced_table").get_to(fk.referenced_table);
    j.at("referenced_column").get_to(fk.referenced_column);
}

void to_json(json& j, const Column& c) {
    j = json::object();
    j["name"] = c.name_;
    j["type"] = c.type_;
    j["foreign_keys"] = c.foreign_keys_;
}

void from_json(const json& j, Column& c) {
    j.at("name").get_to(c.name_);
    j.at("type").get_to(c.type_);
    if (j.contains("foreign_keys")) {
        j.at("foreign_keys").get_to(c.foreign_keys_);
    }
}

Table::Table(std::string name, const std::vector<std::string>& column_names, const std::vector<std::string>& column_types, const std::vector<ForeignKey>& foreign_keys)
    : name_(std::move(name)) {
    for (size_t i = 0; i < column_names.size(); ++i) {
        columns_.emplace_back(column_names[i], column_types[i]);
    }
    
    for (const auto& fk : foreign_keys) {
        for (auto& column : columns_) {
            if (column.get_name() == fk.column_name) {
                column.add_foreign_key(fk);
                break;
            }
        }
    }
}

void Table::insert(const Row& row) {
    rows_.push_back(row);
}

void to_json(json& j, const Table& t) {
    j = json::object();
    j["name"] = t.name_;
    j["columns"] = t.columns_;
    j["rows"] = t.rows_;
}

void from_json(const json& j, Table& t) {
    j.at("name").get_to(t.name_);
    j.at("columns").get_to(t.columns_);
    j.at("rows").get_to(t.rows_);
}

} // namespace db
