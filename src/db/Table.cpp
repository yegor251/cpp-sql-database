#include "db/Table.hpp"

namespace db {

void to_json(json& j, const Column& c) {
    j = json::object();
    j["name"] = c.name;
    j["type"] = c.type;
}

void from_json(const json& j, Column& c) {
    j.at("name").get_to(c.name);
    j.at("type").get_to(c.type);
}

Table::Table(const std::string& name, const std::vector<std::string>& column_names, const std::vector<std::string>& column_types)
    : name(name) {
    for (size_t i = 0; i < column_names.size(); ++i) {
        columns.emplace_back(column_names[i], column_types[i]);
    }
}

void Table::insert(const Row& row) {
    rows.push_back(row);
}

void to_json(json& j, const Table& t) {
    j = json::object();
    j["name"] = t.name;
    j["columns"] = t.columns;
    j["rows"] = t.rows;
}

void from_json(const json& j, Table& t) {
    j.at("name").get_to(t.name);
    j.at("columns").get_to(t.columns);
    j.at("rows").get_to(t.rows);
}

} // namespace db
