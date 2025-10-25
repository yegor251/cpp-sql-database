#include "sql/executors/SelectExecutor.hpp"
#include "db/ValueUtils.hpp"
#include "sql/parsers/Utils.hpp"
#include <algorithm>

namespace sql {
namespace executors {

ExecResult execute_select(const Select& cmd, db::StorageEngine& engine, const std::string& current_db) {
    if (current_db.empty()) return {false, "No database selected", ""};
    
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

}
}
