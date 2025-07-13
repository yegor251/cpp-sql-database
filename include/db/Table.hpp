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

void to_json(json& j, const Column& c);
void from_json(const json& j, Column& c);

struct Table {
    std::string name;
    std::vector<Column> columns;
    std::vector<Row> rows;

    Table() = default;
    Table(const std::string& name, const std::vector<std::string>& column_names, const std::vector<std::string>& column_types);

    void insert(const Row& row);

    friend void to_json(json& j, const Table& t);
    friend void from_json(const json& j, Table& t);
};

void to_json(json& j, const Table& t);
void from_json(const json& j, Table& t);

} // namespace db