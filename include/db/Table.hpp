#pragma once
#include <string>
#include <vector>
#include "db/Row.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace db {

struct Column {
    std::string name;
    std::string type;
    
    Column() = default;
    Column(const std::string& name, const std::string& type) : name(name), type(type) {}
    
    friend void to_json(json& j, const Column& c);
    friend void from_json(const json& j, Column& c);
};

inline void to_json(json& j, const Column& c) {
    j = json::object();
    j["name"] = c.name;
    j["type"] = c.type;
}

inline void from_json(const json& j, Column& c) {
    j.at("name").get_to(c.name);
    j.at("type").get_to(c.type);
}

struct Table {
    std::string name;
    std::vector<Column> columns;
    std::vector<Row> rows;

    Table() = default;

    Table(const std::string& name, const std::vector<std::string>& column_names, const std::vector<std::string>& column_types)
        : name(name) {
        for (size_t i = 0; i < column_names.size(); ++i) {
            columns.emplace_back(column_names[i], column_types[i]);
        }
    }

    void insert(const Row& row) {
        rows.push_back(row);
    }

    friend void to_json(json& j, const Table& t);
    friend void from_json(const json& j, Table& t);
};

inline void to_json(json& j, const Table& t) {
    j = json::object();
    j["name"] = t.name;
    j["columns"] = t.columns;
    j["rows"] = t.rows;
}

inline void from_json(const json& j, Table& t) {
    j.at("name").get_to(t.name);
    j.at("columns").get_to(t.columns);
    j.at("rows").get_to(t.rows);
}

} // namespace db