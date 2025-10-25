#include "sql/executors/UseExecutor.hpp"
#include "db/ValueUtils.hpp"
#include "sql/parsers/Utils.hpp"

namespace sql {
namespace executors {

ExecResult execute_use(const Use& cmd, db::StorageEngine& engine, std::string& current_db) {
    if (!engine.get_database(cmd.db_name))
        return {false, "Database not found", ""};
    current_db = cmd.db_name;
    return {true, "", ""};
}

}
}
