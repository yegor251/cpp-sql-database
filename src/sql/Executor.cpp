#include "sql/Executor.hpp"
#include <variant>
#include <algorithm>
#include <iostream>
#include <cstdlib>

using namespace sql;

std::string Executor::current_db = "";

// Обновляем функцию проверки типов для работы с db::Value
bool validate_type(const db::Value& value, const std::string& expected_type) {
    if (expected_type == "INT") {
        return std::holds_alternative<int>(value);
    }
    if (expected_type == "FLOAT") {
        return std::holds_alternative<float>(value);
    }
    if (expected_type == "BOOL") {
        return std::holds_alternative<bool>(value);
    }
    if (expected_type == "STR") {
        return std::holds_alternative<std::string>(value);
    }
    return false;
}

// Добавляем функцию для конвертации Value в строку для отладки
std::string value_to_string(const db::Value& value) {
    if (std::holds_alternative<int>(value)) {
        return std::to_string(std::get<int>(value));
    } else if (std::holds_alternative<float>(value)) {
        return std::to_string(std::get<float>(value));
    } else if (std::holds_alternative<bool>(value)) {
        return std::get<bool>(value) ? "true" : "false";
    } else if (std::holds_alternative<std::string>(value)) {
        return std::get<std::string>(value);
    }
    return "unknown";
}

ExecResult Executor::execute(const ParseResult& pr, db::StorageEngine& engine) {
    try {
        switch (pr.type) {
        case CommandType::CREATE_DATABASE: {
            auto& cmd = std::get<CreateDatabase>(pr.command);
            engine.create_database(cmd.db_name);
            return {true, ""};
        }
        case CommandType::DROP_DATABASE: {
            auto& cmd = std::get<DropDatabase>(pr.command);
            for (auto& n : cmd.names) engine.drop_database(n);
            if (!current_db.empty() && engine.databases.find(current_db) == engine.databases.end())
                current_db.clear();
            return {true, ""};
        }
        case CommandType::USE: {
            auto& cmd = std::get<Use>(pr.command);
            if (engine.databases.find(cmd.db_name) == engine.databases.end())
                return {false, "Database not found"};
            current_db = cmd.db_name;
            return {true, ""};
        }
        case CommandType::CREATE_TABLE: {
            if (current_db.empty()) return {false, "No database selected"};
            auto& cmd = std::get<CreateTable>(pr.command);
            auto* db = engine.get_database(current_db);
            if (!db) return {false, "Database not found"};
            db->create_table(cmd.table_name, cmd.columns, cmd.types);
            return {true, ""};
        }
        case CommandType::DROP_TABLE: {
            if (current_db.empty()) return {false, "No database selected"};
            auto& cmd = std::get<DropTable>(pr.command);
            auto* db = engine.get_database(current_db);
            if (!db) return {false, "Database not found"};
            for (auto& n : cmd.names) db->drop_table(n);
            return {true, ""};
        }
        case CommandType::INSERT: {
            if (current_db.empty()) return {false, "No database selected"};
            auto& cmd = std::get<Insert>(pr.command);
            auto* db = engine.get_database(current_db);
            if (!db) return {false, "Database not found"};
            
            auto it = db->tables.find(cmd.table_name);
            if (it == db->tables.end()) return {false, "Table not found"};
            
            // Определяем какие колонки и в каком порядке заполнять
            std::vector<const db::Column*> target_columns;
            if (!cmd.columns.empty()) {
                // Если указаны колонки, проверяем их существование и порядок
                if (cmd.columns.size() != cmd.values.size()) {
                    return {false, "Column count doesn't match value count"};
                }
                
                for (const auto& col_name : cmd.columns) {
                    bool found = false;
                    for (const auto& col : it->second.columns) {
                        if (col.name == col_name) {
                            target_columns.push_back(&col);
                            found = true;
                            break;
                        }
                    }
                    if (!found) return {false, "Column '" + col_name + "' not found in table"};
                }
            } else {
                // Если колонки не указаны, берем все по порядку
                if (cmd.values.size() != it->second.columns.size()) {
                    return {false, "Value count doesn't match column count"};
                }
                for (const auto& col : it->second.columns) {
                    target_columns.push_back(&col);
                }
            }
            
            // Проверяем типы всех значений
            for (size_t i = 0; i < target_columns.size(); ++i) {
                const db::Value& value = cmd.values[i];  // Изменено с std::string на db::Value
                const std::string& expected_type = target_columns[i]->type;
                
                if (!validate_type(value, expected_type)) {
                    return {false, "Type mismatch for column '" + target_columns[i]->name + 
                                 "': expected " + expected_type + ", got value '" + value_to_string(value) + "'"};
                }
            }
            
            db::Row row;
            row.values = cmd.values;  // Теперь это vector<db::Value>
            it->second.insert(row);
            return {true, ""};
        }
        case CommandType::SELECT: {
            if (current_db.empty()) return {false, "No database selected"};
            auto& cmd = std::get<Select>(pr.command);
            auto* db = engine.get_database(current_db);
            if (!db) return {false, "Database not found"};
            
            auto it = db->tables.find(cmd.table_name);
            if (it == db->tables.end()) return {false, "Table not found"};
            
            // Проверяем колонки
            if (cmd.columns.size() == 1 && cmd.columns[0] == "*") {
                // SELECT * - все колонки
            } else {
                for (const auto& col_name : cmd.columns) {
                    bool found = false;
                    for (const auto& col : it->second.columns) {
                        if (col.name == col_name) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) return {false, "Column '" + col_name + "' not found in table"};
                }
            }
            
            // Проверяем WHERE если есть
            if (!cmd.where.empty()) {
                if (cmd.where.size() >= 3) {
                    std::string col_name = cmd.where[0];
                    std::string op = cmd.where[1];
                    std::string value = cmd.where[2];
                    
                    // Проверяем что колонка существует
                    const db::Column* target_col = nullptr;
                    for (const auto& col : it->second.columns) {
                        if (col.name == col_name) {
                            target_col = &col;
                            break;
                        }
                    }
                    if (!target_col) return {false, "Column '" + col_name + "' not found in WHERE clause"};
                    
                    // Для WHERE пока оставляем строковую проверку, так как WHERE парсится как строки
                    if (!validate_type(db::Value(value), target_col->type)) {
                        return {false, "Type mismatch in WHERE clause for column '" + col_name + 
                                     "': expected " + target_col->type + ", got value '" + value + "'"};
                    }
                }
            }
            
            return {true, ""};
        }
        case CommandType::UPDATE: {
            if (current_db.empty()) return {false, "No database selected"};
            return {false, "UPDATE not implemented"};
        }
        case CommandType::DELETE: {
            if (current_db.empty()) return {false, "No database selected"};
            auto& cmd = std::get<Delete>(pr.command);
            auto* db = engine.get_database(current_db);
            if (!db) return {false, "Database not found"};
            
            auto it = db->tables.find(cmd.table_name);
            if (it == db->tables.end()) return {false, "Table not found"};
            
            // Проверяем WHERE если есть
            if (!cmd.where.empty()) {
                if (cmd.where.size() >= 3) {
                    std::string col_name = cmd.where[0];
                    std::string op = cmd.where[1];
                    std::string value = cmd.where[2];
                    
                    // Проверяем что колонка существует
                    const db::Column* target_col = nullptr;
                    for (const auto& col : it->second.columns) {
                        if (col.name == col_name) {
                            target_col = &col;
                            break;
                        }
                    }
                    if (!target_col) return {false, "Column '" + col_name + "' not found in WHERE clause"};
                    
                    // Для WHERE пока оставляем строковую проверку
                    if (!validate_type(db::Value(value), target_col->type)) {
                        return {false, "Type mismatch in WHERE clause for column '" + col_name + 
                                     "': expected " + target_col->type + ", got value '" + value + "'"};
                    }
                }
            }
            
            return {false, "DELETE not implemented"};
        }
        default:
            return {false, "Unsupported command"};
        }
    } catch (const std::exception& e) {
        return {false, e.what()};
    }
}
