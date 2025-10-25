#include "sql/executors/DropExecutor.hpp"
#include "db/ValueUtils.hpp"
#include "sql/parsers/Utils.hpp"

namespace sql {
namespace executors {

ExecResult execute_drop_database(const DropDatabase& cmd, db::StorageEngine& engine, std::string& current_db) {
    for (const auto& n : cmd.names) engine.drop_database(n);
    if (!current_db.empty() && !engine.get_database(current_db))
        current_db.clear();
    return {true, "", ""};
}

ExecResult execute_drop_table(const DropTable& cmd, db::StorageEngine& engine, const std::string& current_db) {
    if (current_db.empty()) return {false, "No database selected", ""};
    
    auto* db = engine.get_database(current_db);
    if (!db) return {false, "Database not found", ""};
    
    for (const auto& n : cmd.names) db->drop_table(n);
    return {true, "", ""};
}

}
}
