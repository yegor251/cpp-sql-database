#include "sql/executors/InsertExecutor.hpp"
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

ExecResult execute_insert(const Insert& cmd, db::StorageEngine& engine, const std::string& current_db) {
    if (current_db.empty()) return {false, "No database selected", ""};
    
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

}
}
