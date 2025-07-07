#pragma once
#include <string>
#include <vector>
#include <variant>
#include "db/Row.hpp"

namespace sql {

enum class CommandType {
    CREATE_DATABASE,
    DROP_DATABASE,
    CREATE_TABLE,
    DROP_TABLE,
    INSERT,
    SELECT,
    UPDATE,
    DELETE,
    USE,
    UNKNOWN
};

struct CreateDatabase {
    std::string db_name;
};

struct DropDatabase {
    std::vector<std::string> names;
};

struct CreateTable {
    std::string table_name;
    std::vector<std::string> columns;
    std::vector<std::string> types;
    std::vector<std::string> primary_keys;
    std::vector<std::string> foreign_keys;
    std::vector<std::string> constraints;
};

struct DropTable {
    std::vector<std::string> names;
};

struct Insert {
    std::string table_name;
    std::vector<std::string> columns;
    std::vector<db::Value> values;
};

struct Select {
    std::vector<std::string> columns;
    std::string table_name;
    std::vector<std::string> where;
};

struct Update {
    std::string table_name;
    std::vector<std::string> set;
    std::vector<std::string> where;
};

struct Delete {
    std::string table_name;
    std::vector<std::string> where;
};

struct Use {
    std::string db_name;
};

using Command = std::variant<
    CreateDatabase,
    DropDatabase,
    CreateTable,
    DropTable,
    Insert,
    Select,
    Update,
    Delete,
    Use
>;

struct ParseResult {
    CommandType type;
    Command command;
    bool valid;
    std::string error;
};

} // namespace sql
