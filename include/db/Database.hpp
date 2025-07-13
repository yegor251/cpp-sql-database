#pragma once
#include <string>
#include <unordered_map>
#include "db/Table.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace db {

struct Database {
    std::string name;
    std::unordered_map<std::string, Table> tables;

    Database() = default;
    Database(const std::string& name) : name(name) {}

    void create_table(const std::string& table_name, const std::vector<std::string>& columns, const std::vector<std::string>& types);
    void drop_table(const std::string& table_name);

    friend void to_json(json& j, const Database& d);
    friend void from_json(const json& j, Database& d);
};

void to_json(json& j, const Database& d);
void from_json(const json& j, Database& d);

} // namespace db