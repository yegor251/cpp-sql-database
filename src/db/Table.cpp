#include "db/Table.hpp"

namespace db {

void to_json(json& j, const Column& c) {
    j = json::object();
    j["name"] = c.name_;
    j["type"] = c.type_;
}

void from_json(const json& j, Column& c) {
    j.at("name").get_to(c.name_);
    j.at("type").get_to(c.type_);
}

Table::Table(std::string name, const std::vector<std::string>& column_names, const std::vector<std::string>& column_types)
    : name_(std::move(name)) {
    for (size_t i = 0; i < column_names.size(); ++i) {
        columns_.emplace_back(column_names[i], column_types[i]);
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
