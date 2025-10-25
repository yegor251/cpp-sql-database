#pragma once
#include "sql/AST.hpp"
#include "sql/Executor.hpp"
#include "db/StorageEngine.hpp"

namespace sql {
namespace executors {

ExecResult execute_use(const Use& cmd, db::StorageEngine& engine, std::string& current_db);

}
}
