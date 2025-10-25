#pragma once
#include "sql/AST.hpp"
#include "sql/Executor.hpp"
#include "db/StorageEngine.hpp"

namespace sql {
namespace executors {

ExecResult execute_drop_database(const DropDatabase& cmd, db::StorageEngine& engine, std::string& current_db);
ExecResult execute_drop_table(const DropTable& cmd, db::StorageEngine& engine, const std::string& current_db);

}
}
