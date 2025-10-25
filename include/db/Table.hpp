#pragma once
#include <string>
#include <vector>
#include <string_view>
#include "db/Row.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace db {

struct ForeignKey {
    std::string column_name;
    std::string referenced_table;
    std::string referenced_column;
    
    ForeignKey() = default;
    ForeignKey(std::string col, std::string ref_table, std::string ref_col)
        : column_name(std::move(col)), referenced_table(std::move(ref_table)), referenced_column(std::move(ref_col)) {}
    
    friend void to_json(json& j, const ForeignKey& fk);
    friend void from_json(const json& j, ForeignKey& fk);
};

void to_json(json& j, const ForeignKey& fk);
void from_json(const json& j, ForeignKey& fk);

class Column {
public:
    Column() = default;
    Column(std::string name, std::string type)
        : name_(std::move(name)), type_(std::move(type)) {}

    const std::string& get_name() const noexcept { return name_; }
    const std::string& get_type() const noexcept { return type_; }
    const std::vector<ForeignKey>& get_foreign_keys() const noexcept { return foreign_keys_; }
    std::vector<ForeignKey>& get_foreign_keys() noexcept { return foreign_keys_; }

    void add_foreign_key(const ForeignKey& fk) { foreign_keys_.push_back(fk); }

    friend void to_json(json& j, const Column& c);
    friend void from_json(const json& j, Column& c);

private:
    std::string name_;
    std::string type_;
    std::vector<ForeignKey> foreign_keys_;
};

void to_json(json& j, const Column& c);
void from_json(const json& j, Column& c);

class Table {
public:
    Table() = default;
    Table(std::string name, const std::vector<std::string>& column_names, const std::vector<std::string>& column_types, const std::vector<ForeignKey>& foreign_keys = {});

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

}