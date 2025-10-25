#pragma once
#include "sql/AST.hpp"
#include "sql/Executor.hpp"
#include "db/StorageEngine.hpp"

namespace sql {
namespace executors {

ExecResult execute_select(const Select& cmd, db::StorageEngine& engine, const std::string& current_db);

}
}
