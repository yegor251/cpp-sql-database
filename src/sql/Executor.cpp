#include "sql/Executor.hpp"
#include "db/ValueUtils.hpp"
#include <variant>
#include <algorithm>
#include <iostream>
#include <cstdlib>

using namespace sql;

std::string Executor::current_db = "";

namespace {

bool validate_type(const db::Value& value, const std::string& expected_type) {
    if (expected_type == "INT") return std::holds_alternative<int>(value);
    if (expected_type == "FLOAT") return std::holds_alternative<float>(value);
    if (expected_type == "BOOL") return std::holds_alternative<bool>(value);
    if (expected_type == "STR") return std::holds_alternative<std::string>(value);
    return false;
}

}

ExecResult Executor::execute(const ParseResult& pr, db::StorageEngine& engine) {
    try {
        switch (pr.type) {
        case CommandType::CREATE_DATABASE: {
            const auto& cmd = std::get<CreateDatabase>(pr.command);
            engine.create_database(cmd.db_name);
            return {true, "", ""};
        }
        case CommandType::DROP_DATABASE: {
            const auto& cmd = std::get<DropDatabase>(pr.command);
            for (const auto& n : cmd.names) engine.drop_database(n);
            if (!current_db.empty() && !engine.get_database(current_db))
                current_db.clear();
            return {true, "", ""};
        }
        case CommandType::USE: {
            const auto& cmd = std::get<Use>(pr.command);
            if (!engine.get_database(cmd.db_name))
                return {false, "Database not found", ""};
            current_db = cmd.db_name;
            return {true, "", ""};
        }
        case CommandType::CREATE_TABLE: {
            if (current_db.empty()) return {false, "No database selected", ""};
            const auto& cmd = std::get<CreateTable>(pr.command);
            auto* db = engine.get_database(current_db);
            if (!db) return {false, "Database not found", ""};
            db->create_table(cmd.table_name, cmd.columns, cmd.types);
            return {true, "", ""};
        }
        case CommandType::DROP_TABLE: {
            if (current_db.empty()) return {false, "No database selected", ""};
            const auto& cmd = std::get<DropTable>(pr.command);
            auto* db = engine.get_database(current_db);
            if (!db) return {false, "Database not found", ""};
            for (const auto& n : cmd.names) db->drop_table(n);
            return {true, "", ""};
        }
        case CommandType::INSERT: {
            if (current_db.empty()) return {false, "No database selected", ""};
            const auto& cmd = std::get<Insert>(pr.command);
            auto* db = engine.get_database(current_db);
            if (!db) return {false, "Database not found", ""};
            auto* table = db->get_table(cmd.table_name);
            if (!table) return {false, "Table not found", ""};

            std::vector<const db::Column*> target_columns;
            const auto& columns = table->get_columns();
            if (!cmd.columns.empty()) {
                if (cmd.columns.size() != cmd.values.size())
                    return {false, "Column count doesn't match value count", ""};
                for (const auto& col_name : cmd.columns) {
                    auto it = std::find_if(columns.begin(), columns.end(),
                        [&](const db::Column& col) { return col.get_name() == col_name; });
                    if (it == columns.end())
                        return {false, "Column '" + col_name + "' not found in table", ""};
                    target_columns.push_back(&(*it));
                }
            } else {
                if (cmd.values.size() != columns.size())
                    return {false, "Value count doesn't match column count", ""};
                for (const auto& col : columns)
                    target_columns.push_back(&col);
            }

            for (size_t i = 0; i < target_columns.size(); ++i) {
                const db::Value& value = cmd.values[i];
                const std::string& expected_type = target_columns[i]->get_type();
                if (!validate_type(value, expected_type)) {
                    return {false, "Type mismatch for column '" + target_columns[i]->get_name() +
                                   "': expected " + expected_type + ", got value '" + db::value_to_string(value) + "'", ""};
                }
            }

            table->insert(db::Row(cmd.values));
            return {true, "", ""};
        }
        case CommandType::SELECT: {
            if (current_db.empty()) return {false, "No database selected", ""};
            const auto& cmd = std::get<Select>(pr.command);
            auto* db = engine.get_database(current_db);
            if (!db) return {false, "Database not found", ""};
            auto* table = db->get_table(cmd.table_name);
            if (!table) return {false, "Table not found", ""};

            const auto& columns = table->get_columns();
            std::vector<const db::Column*> selected_columns;
            
            if (cmd.columns.size() == 1 && cmd.columns[0] == "*") {
                for (const auto& col : columns) {
                    selected_columns.push_back(&col);
                }
            } else {
                for (const auto& col_name : cmd.columns) {
                    auto it = std::find_if(columns.begin(), columns.end(),
                        [&](const db::Column& col) { return col.get_name() == col_name; });
                    if (it == columns.end())
                        return {false, "Column '" + col_name + "' not found in table", ""};
                    selected_columns.push_back(&(*it));
                }
            }

            std::string result = "";
            for (size_t i = 0; i < selected_columns.size(); ++i) {
                if (i > 0) result += " | ";
                result += selected_columns[i]->get_name();
            }
            result += "\n";
            
            for (size_t i = 0; i < selected_columns.size(); ++i) {
                if (i > 0) result += "-+-";
                for (size_t j = 0; j < selected_columns[i]->get_name().length(); ++j) {
                    result += "-";
                }
            }
            result += "\n";

            const auto& rows = table->get_rows();
            for (const auto& row : rows) {
                const auto& row_values = row.get_values();
                for (size_t i = 0; i < selected_columns.size(); ++i) {
                    if (i > 0) result += " | ";
                    size_t col_index = 0;
                    for (size_t j = 0; j < columns.size(); ++j) {
                        if (columns[j].get_name() == selected_columns[i]->get_name()) {
                            col_index = j;
                            break;
                        }
                    }
                    result += db::value_to_string(row_values[col_index]);
                }
                result += "\n";
            }

            return {true, "", result};
        }
        case CommandType::UPDATE: {
            if (current_db.empty()) return {false, "No database selected", ""};
            return {false, "UPDATE not implemented", ""};
        }
        case CommandType::DELETE: {
            if (current_db.empty()) return {false, "No database selected", ""};
            const auto& cmd = std::get<Delete>(pr.command);
            auto* db = engine.get_database(current_db);
            if (!db) return {false, "Database not found", ""};
            auto* table = db->get_table(cmd.table_name);
            if (!table) return {false, "Table not found", ""};
            return {false, "DELETE not implemented", ""};
        }
        default:
            return {false, "Unsupported command", ""};
        }
    } catch (const std::exception& e) {
        return {false, e.what(), ""};
    }
}
