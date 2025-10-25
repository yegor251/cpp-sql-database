#include "sql/executors/DeleteExecutor.hpp"
#include "db/ValueUtils.hpp"
#include "sql/parsers/Utils.hpp"
#include <algorithm>

namespace sql {
namespace executors {

ExecResult execute_delete(const Delete& cmd, db::StorageEngine& engine, const std::string& current_db) {
    if (current_db.empty()) return {false, "No database selected", ""};
    
    auto* db = engine.get_database(current_db);
    if (!db) return {false, "Database not found", ""};
    
    auto* table = db->get_table(cmd.table_name);
    if (!table) return {false, "Table not found", ""};
    
    auto& rows = table->get_rows();
    const auto& columns = table->get_columns();
    
    if (cmd.where.empty()) {
        const auto& all_tables = db->get_tables();
        for (const auto& [other_table_name, other_table] : all_tables) {
            if (other_table_name == cmd.table_name) continue;
            
            for (const auto& other_column : other_table.get_columns()) {
                for (const auto& fk : other_column.get_foreign_keys()) {
                    if (fk.referenced_table == cmd.table_name) {
                        for (const auto& other_row : other_table.get_rows()) {
                            const auto& other_row_values = other_row.get_values();
                            for (size_t j = 0; j < other_table.get_columns().size(); ++j) {
                                if (other_table.get_columns()[j].get_name() == fk.column_name) {
                                    if (j < other_row_values.size()) {
                                        int ref_column_index = -1;
                                        for (size_t k = 0; k < columns.size(); ++k) {
                                            if (columns[k].get_name() == fk.referenced_column) {
                                                ref_column_index = k;
                                                break;
                                            }
                                        }
                                        
                                        if (ref_column_index != -1) {
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
            const auto& row_values = it->get_values();
            bool is_referenced = false;
            std::string reference_info = "";
            
            const auto& all_tables = db->get_tables();
            for (const auto& [other_table_name, other_table] : all_tables) {
                if (other_table_name == cmd.table_name) continue;
                
                for (const auto& other_column : other_table.get_columns()) {
                    for (const auto& fk : other_column.get_foreign_keys()) {
                        if (fk.referenced_table == cmd.table_name) {
                            for (const auto& other_row : other_table.get_rows()) {
                                const auto& other_row_values = other_row.get_values();
                                for (size_t j = 0; j < other_table.get_columns().size(); ++j) {
                                    if (other_table.get_columns()[j].get_name() == fk.column_name) {
                                        if (j < other_row_values.size()) {
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

}
}
