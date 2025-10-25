#include "sql/executors/UpdateExecutor.hpp"
#include "db/ValueUtils.hpp"
#include "sql/parsers/Utils.hpp"
#include <algorithm>

namespace sql {
namespace executors {

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

ExecResult execute_update(const Update& cmd, db::StorageEngine& engine, const std::string& current_db) {
    if (current_db.empty()) return {false, "No database selected", ""};
    
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
            const auto& current_row_values = row.get_values();
            for (const auto& update : updates) {
                const std::string& column_name = update.first;
                const db::Value& new_value = update.second;
                
                int column_index = -1;
                for (size_t i = 0; i < columns.size(); ++i) {
                    if (columns[i].get_name() == column_name) {
                        column_index = i;
                        break;
                    }
                }
                
                if (column_index != -1 && column_index < current_row_values.size()) {
                    const db::Value& current_value = current_row_values[column_index];
                    
                    const auto& all_tables = db->get_tables();
                    for (const auto& [other_table_name, other_table] : all_tables) {
                        if (other_table_name == cmd.table_name) continue;
                        
                        for (const auto& other_column : other_table.get_columns()) {
                            for (const auto& fk : other_column.get_foreign_keys()) {
                                if (fk.referenced_table == cmd.table_name && fk.referenced_column == column_name) {
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

}
}
