#pragma once
#include <string>
#include <vector>
#include <string_view>
#include "db/Row.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace db {

class Column {
public:
    Column() = default;
    Column(std::string name, std::string type)
        : name_(std::move(name)), type_(std::move(type)) {}

    const std::string& get_name() const noexcept { return name_; }
    const std::string& get_type() const noexcept { return type_; }

    friend void to_json(json& j, const Column& c);
    friend void from_json(const json& j, Column& c);

private:
    std::string name_;
    std::string type_;
};

void to_json(json& j, const Column& c);
void from_json(const json& j, Column& c);

class Table {
public:
    Table() = default;
    Table(std::string name, const std::vector<std::string>& column_names, const std::vector<std::string>& column_types);

    void insert(const Row& row);

    const std::string& get_name() const noexcept { return name_; }
    const std::vector<Column>& get_columns() const noexcept { return columns_; }
    const std::vector<Row>& get_rows() const noexcept { return rows_; }
    std::vector<Row>& get_rows() noexcept { return rows_; }

    friend void to_json(json& j, const Table& t);
    friend void from_json(const json& j, Table& t);

private:
    std::string name_;
    std::vector<Column> columns_;
    std::vector<Row> rows_;
};

void to_json(json& j, const Table& t);
void from_json(const json& j, Table& t);

} // namespace db