#include "sql/Executor.hpp"
#include "db/ValueUtils.hpp"
#include "sql/parsers/Utils.hpp"
#include <variant>
#include <algorithm>
#include <iostream>
#include <cstdlib>

using namespace sql;

std::string Executor::current_db = "";

namespace {

bool validate_type(const db::Value& value, const std::string& expected_type) {
    if (std::holds_alternative<db::NullValue>(value)) return true;
    
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
            
            for (const auto& fk : cmd.foreign_keys) {
                const auto* ref_table = db->get_table(fk.referenced_table);
                if (!ref_table) {
                    return {false, "Referenced table '" + fk.referenced_table + "' does not exist", ""};
                }
                
                bool ref_column_exists = false;
                for (const auto& col : ref_table->get_columns()) {
                    if (col.get_name() == fk.referenced_column) {
                        ref_column_exists = true;
                        break;
                    }
                }
                if (!ref_column_exists) {
                    return {false, "Referenced column '" + fk.referenced_column + "' does not exist in table '" + fk.referenced_table + "'", ""};
                }
                
                bool fk_column_exists = false;
                for (const auto& col_name : cmd.columns) {
                    if (col_name == fk.column_name) {
                        fk_column_exists = true;
                        break;
                    }
                }
                if (!fk_column_exists) {
                    return {false, "Foreign key column '" + fk.column_name + "' does not exist in table", ""};
                }
            }
            
            std::vector<db::ForeignKey> db_foreign_keys;
            for (const auto& fk : cmd.foreign_keys) {
                db_foreign_keys.emplace_back(fk.column_name, fk.referenced_table, fk.referenced_column);
            }
            
            db->create_table(cmd.table_name, cmd.columns, cmd.types, db_foreign_keys);
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

            for (size_t i = 0; i < target_columns.size(); ++i) {
                const auto& column = *target_columns[i];
                const auto& value = cmd.values[i];
                
                if (std::holds_alternative<db::NullValue>(value)) {
                    continue;
                }
                
                for (const auto& fk : column.get_foreign_keys()) {
                    const auto* ref_table = db->get_table(fk.referenced_table);
                    if (!ref_table) {
                        return {false, "Referenced table '" + fk.referenced_table + "' not found for foreign key constraint", ""};
                    }
                    
                    bool value_exists = false;
                    for (const auto& row : ref_table->get_rows()) {
                        const auto& row_values = row.get_values();
                        for (size_t j = 0; j < ref_table->get_columns().size(); ++j) {
                            if (ref_table->get_columns()[j].get_name() == fk.referenced_column) {
                                if (j < row_values.size() && db::value_equals(row_values[j], value)) {
                                    value_exists = true;
                                    break;
                                }
                            }
                        }
                        if (value_exists) break;
                    }
                    
                    if (!value_exists) {
                        return {false, "Foreign key constraint violation: value '" + db::value_to_string(value) + 
                                       "' does not exist in referenced table '" + fk.referenced_table + 
                                       "' column '" + fk.referenced_column + "'", ""};
                    }
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
            const auto& cmd = std::get<Update>(pr.command);
            auto* db = engine.get_database(current_db);
            if (!db) return {false, "Database not found", ""};
            auto* table = db->get_table(cmd.table_name);
            if (!table) return {false, "Table not found", ""};

            const auto& columns = table->get_columns();
            auto& rows = table->get_rows();

            std::vector<std::pair<std::string, db::Value>> updates;
            for (const auto& set_clause : cmd.set) {
                size_t equals_pos = set_clause.find('=');
                if (equals_pos == std::string::npos) {
                    return {false, "Invalid SET clause: " + set_clause, ""};
                }
                
                std::string column_name = set_clause.substr(0, equals_pos);
                std::string value_str = set_clause.substr(equals_pos + 1);
                
                int column_index = -1;
                for (size_t i = 0; i < columns.size(); ++i) {
                    if (columns[i].get_name() == column_name) {
                        column_index = i;
                        break;
                    }
                }
                
                if (column_index == -1) {
                    return {false, "Column '" + column_name + "' not found in table", ""};
                }
                
                db::Value value = sql::parsers::parse_value(value_str);
                
                const std::string& expected_type = columns[column_index].get_type();
                if (!validate_type(value, expected_type)) {
                    return {false, "Type mismatch for column '" + column_name + 
                                   "': expected " + expected_type + ", got value '" + db::value_to_string(value) + "'", ""};
                }
                
                const auto& column = columns[column_index];
                for (const auto& fk : column.get_foreign_keys()) {
                    if (std::holds_alternative<db::NullValue>(value)) {
                        continue;
                    }
                    
                    const auto* ref_table = db->get_table(fk.referenced_table);
                    if (!ref_table) {
                        return {false, "Referenced table '" + fk.referenced_table + "' not found for foreign key constraint", ""};
                    }
                    
                    bool value_exists = false;
                    for (const auto& row : ref_table->get_rows()) {
                        const auto& row_values = row.get_values();
                        for (size_t j = 0; j < ref_table->get_columns().size(); ++j) {
                            if (ref_table->get_columns()[j].get_name() == fk.referenced_column) {
                                if (j < row_values.size() && db::value_equals(row_values[j], value)) {
                                    value_exists = true;
                                    break;
                                }
                            }
                        }
                        if (value_exists) break;
                    }
                    
                    if (!value_exists) {
                        return {false, "Foreign key constraint violation: value '" + db::value_to_string(value) + 
                                       "' does not exist in referenced table '" + fk.referenced_table + 
                                       "' column '" + fk.referenced_column + "'", ""};
                    }
                }
                
                updates.push_back({column_name, value});
            }
            
            // FK reference check will be done for each row during update
            
            int updated_count = 0;
            for (auto& row : rows) {
                bool should_update = true;
                
                if (!cmd.where.empty()) {
                    should_update = false;
                    for (const auto& where_clause : cmd.where) {
                        size_t equals_pos = where_clause.find('=');
                        if (equals_pos != std::string::npos) {
                            std::string where_column = where_clause.substr(0, equals_pos);
                            std::string where_value_str = where_clause.substr(equals_pos + 1);
                            
                            int where_column_index = -1;
                            for (size_t i = 0; i < columns.size(); ++i) {
                                if (columns[i].get_name() == where_column) {
                                    where_column_index = i;
                                    break;
                                }
                            }
                            
                            if (where_column_index != -1) {
                                db::Value where_value = sql::parsers::parse_value(where_value_str);
                                const auto& row_values = row.get_values();
                                if (where_column_index < row_values.size() && 
                                    db::value_equals(row_values[where_column_index], where_value)) {
                                    should_update = true;
                                    break;
                                }
                            }
                        }
                    }
                }
                
                if (should_update) {
                    // Check if any values being updated are referenced by other tables
                    const auto& current_row_values = row.get_values();
                    for (const auto& update : updates) {
                        const std::string& column_name = update.first;
                        const db::Value& new_value = update.second;
                        
                        // Find the current value in this row
                        int column_index = -1;
                        for (size_t i = 0; i < columns.size(); ++i) {
                            if (columns[i].get_name() == column_name) {
                                column_index = i;
                                break;
                            }
                        }
                        
                        if (column_index != -1 && column_index < current_row_values.size()) {
                            const db::Value& current_value = current_row_values[column_index];
                            
                            // Check if this current value is referenced by other tables
                            const auto& all_tables = db->get_tables();
                            for (const auto& [other_table_name, other_table] : all_tables) {
                                if (other_table_name == cmd.table_name) continue; // Skip self
                                
                                for (const auto& other_column : other_table.get_columns()) {
                                    for (const auto& fk : other_column.get_foreign_keys()) {
                                        if (fk.referenced_table == cmd.table_name && fk.referenced_column == column_name) {
                                            // Check if any row in this table references the current value
                                            for (const auto& other_row : other_table.get_rows()) {
                                                const auto& other_row_values = other_row.get_values();
                                                for (size_t j = 0; j < other_table.get_columns().size(); ++j) {
                                                    if (other_table.get_columns()[j].get_name() == fk.column_name) {
                                                        if (j < other_row_values.size() && 
                                                            db::value_equals(other_row_values[j], current_value)) {
                                                            return {false, "Cannot update row: value '" + db::value_to_string(current_value) + 
                                                                           "' in column '" + column_name + 
                                                                           "' is referenced by table '" + other_table_name + 
                                                                           "' column '" + fk.column_name + "'", ""};
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    
                    auto& row_values = row.get_values();
                    for (const auto& update : updates) {
                        int update_column_index = -1;
                        for (size_t i = 0; i < columns.size(); ++i) {
                            if (columns[i].get_name() == update.first) {
                                update_column_index = i;
                                break;
                            }
                        }
                        
                        if (update_column_index != -1 && update_column_index < row_values.size()) {
                            row_values[update_column_index] = update.second;
                        }
                    }
                    updated_count++;
                }
            }
            
            return {true, "", "Updated " + std::to_string(updated_count) + " row(s)"};
        }
        case CommandType::DELETE: {
            if (current_db.empty()) return {false, "No database selected", ""};
            const auto& cmd = std::get<Delete>(pr.command);
            auto* db = engine.get_database(current_db);
            if (!db) return {false, "Database not found", ""};
            auto* table = db->get_table(cmd.table_name);
            if (!table) return {false, "Table not found", ""};
            
            auto& rows = table->get_rows();
            const auto& columns = table->get_columns();
            
            if (cmd.where.empty()) {
                // Check if any rows are referenced before clearing all
                const auto& all_tables = db->get_tables();
                for (const auto& [other_table_name, other_table] : all_tables) {
                    if (other_table_name == cmd.table_name) continue; // Skip self
                    
                    for (const auto& other_column : other_table.get_columns()) {
                        for (const auto& fk : other_column.get_foreign_keys()) {
                            if (fk.referenced_table == cmd.table_name) {
                                // This table references our table
                                // Check if any row in this table references any row in our table
                                for (const auto& other_row : other_table.get_rows()) {
                                    const auto& other_row_values = other_row.get_values();
                                    for (size_t j = 0; j < other_table.get_columns().size(); ++j) {
                                        if (other_table.get_columns()[j].get_name() == fk.column_name) {
                                            if (j < other_row_values.size()) {
                                                // Find the referenced column in our table
                                                int ref_column_index = -1;
                                                for (size_t k = 0; k < columns.size(); ++k) {
                                                    if (columns[k].get_name() == fk.referenced_column) {
                                                        ref_column_index = k;
                                                        break;
                                                    }
                                                }
                                                
                                                if (ref_column_index != -1) {
                                                    // Check if any row in our table has this value
                                                    for (const auto& our_row : rows) {
                                                        const auto& our_row_values = our_row.get_values();
                                                        if (ref_column_index < our_row_values.size() &&
                                                            db::value_equals(other_row_values[j], our_row_values[ref_column_index])) {
                                                            return {false, "Cannot delete all rows: table '" + cmd.table_name + 
                                                                           "' is referenced by table '" + other_table_name + 
                                                                           "' column '" + fk.column_name + "'", ""};
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                
                rows.clear();
                return {true, "", "Deleted all rows from table " + cmd.table_name};
            }
            
            int deleted_count = 0;
            auto it = rows.begin();
            
            while (it != rows.end()) {
                bool should_delete = false;
                
                for (size_t i = 0; i < cmd.where.size(); ++i) {
                    if (i + 2 < cmd.where.size() && cmd.where[i+1] == "=") {
                        std::string where_column = cmd.where[i];
                        std::string where_value_str = cmd.where[i+2];
                        
                        int where_column_index = -1;
                        for (size_t j = 0; j < columns.size(); ++j) {
                            if (columns[j].get_name() == where_column) {
                                where_column_index = j;
                                break;
                            }
                        }
                        
                        if (where_column_index != -1) {
                            db::Value where_value = sql::parsers::parse_value(where_value_str);
                            const auto& row_values = it->get_values();
                            
                            if (where_column_index < row_values.size() && 
                                db::value_equals(row_values[where_column_index], where_value)) {
                                should_delete = true;
                                break;
                            }
                        }
                        i += 2;
                    }
                }
                
                if (should_delete) {
                    // Check if this row is referenced by other tables
                    const auto& row_values = it->get_values();
                    bool is_referenced = false;
                    std::string reference_info = "";
                    
                    // Find all tables that might reference this table
                    const auto& all_tables = db->get_tables();
                    for (const auto& [other_table_name, other_table] : all_tables) {
                        if (other_table_name == cmd.table_name) continue; // Skip self
                        
                        for (const auto& other_column : other_table.get_columns()) {
                            for (const auto& fk : other_column.get_foreign_keys()) {
                                if (fk.referenced_table == cmd.table_name) {
                                    // This table references our table
                                    // Check if any row in this table references the row we want to delete
                                    for (const auto& other_row : other_table.get_rows()) {
                                        const auto& other_row_values = other_row.get_values();
                                        for (size_t j = 0; j < other_table.get_columns().size(); ++j) {
                                            if (other_table.get_columns()[j].get_name() == fk.column_name) {
                                                if (j < other_row_values.size()) {
                                                    // Find the referenced column in our table
                                                    int ref_column_index = -1;
                                                    for (size_t k = 0; k < columns.size(); ++k) {
                                                        if (columns[k].get_name() == fk.referenced_column) {
                                                            ref_column_index = k;
                                                            break;
                                                        }
                                                    }
                                                    
                                                    if (ref_column_index != -1 && ref_column_index < row_values.size() &&
                                                        db::value_equals(other_row_values[j], row_values[ref_column_index])) {
                                                        is_referenced = true;
                                                        reference_info = "Referenced by table '" + other_table_name + 
                                                                        "' column '" + fk.column_name + "'";
                                                        break;
                                                    }
                                                }
                                            }
                                        }
                                        if (is_referenced) break;
                                    }
                                    if (is_referenced) break;
                                }
                            }
                            if (is_referenced) break;
                        }
                        if (is_referenced) break;
                    }
                    
                    if (is_referenced) {
                        return {false, "Cannot delete row: " + reference_info, ""};
                    }
                    
                    it = rows.erase(it);
                    deleted_count++;
                } else {
                    ++it;
                }
            }
            
            return {true, "", "Deleted " + std::to_string(deleted_count) + " row(s) from table " + cmd.table_name};
        }
        default:
            return {false, "Unsupported command", ""};
        }
    } catch (const std::exception& e) {
        return {false, e.what(), ""};
    }
}
