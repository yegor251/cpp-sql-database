#pragma once
#include "sql/AST.hpp"
#include "sql/Executor.hpp"
#include "db/StorageEngine.hpp"

namespace sql {
namespace executors {

ExecResult execute_create_database(const CreateDatabase& cmd, db::StorageEngine& engine);
ExecResult execute_create_table(const CreateTable& cmd, db::StorageEngine& engine, const std::string& current_db);

}
}
