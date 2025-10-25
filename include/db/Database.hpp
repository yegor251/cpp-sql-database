#pragma once
#include <string>
#include <unordered_map>
#include <string_view>
#include "db/Table.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace db {

class Database {
public:
    explicit Database(std::string name = {});

    void create_table(std::string_view table_name, const std::vector<std::string>& columns, const std::vector<std::string>& types, const std::vector<ForeignKey>& foreign_keys = {});
    void drop_table(std::string_view table_name);

    [[nodiscard]] const Table* get_table(std::string_view table_name) const noexcept;
    [[nodiscard]] Table* get_table(std::string_view table_name) noexcept;

    const std::string& get_name() const noexcept { return name_; }
    const std::unordered_map<std::string, Table>& get_tables() const noexcept { return tables_; }

    friend void to_json(json& j, const Database& d);
    friend void from_json(const json& j, Database& d);

private:
    std::string name_;
    std::unordered_map<std::string, Table> tables_;
};

void to_json(json& j, const Database& d);
void from_json(const json& j, Database& d);

}