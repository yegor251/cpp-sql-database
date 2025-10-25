#include "sql/executors/CreateExecutor.hpp"
#include "db/ValueUtils.hpp"
#include "sql/parsers/Utils.hpp"
#include <algorithm>

namespace sql {
namespace executors {

ExecResult execute_create_database(const CreateDatabase& cmd, db::StorageEngine& engine) {
    engine.create_database(cmd.db_name);
    return {true, "", ""};
}

ExecResult execute_create_table(const CreateTable& cmd, db::StorageEngine& engine, const std::string& current_db) {
    if (current_db.empty()) return {false, "No database selected", ""};
    
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

}
}
