#include "sql/Executor.hpp"
#include "sql/executors/CreateExecutor.hpp"
#include "sql/executors/DropExecutor.hpp"
#include "sql/executors/UseExecutor.hpp"
#include "sql/executors/InsertExecutor.hpp"
#include "sql/executors/SelectExecutor.hpp"
#include "sql/executors/UpdateExecutor.hpp"
#include "sql/executors/DeleteExecutor.hpp"
#include <variant>
#include <algorithm>
#include <iostream>
#include <cstdlib>

using namespace sql;

std::string Executor::current_db = "";

ExecResult Executor::execute(const ParseResult& pr, db::StorageEngine& engine) {
    try {
        switch (pr.type) {
        case CommandType::CREATE_DATABASE: {
            const auto& cmd = std::get<CreateDatabase>(pr.command);
            return executors::execute_create_database(cmd, engine);
        }
        case CommandType::DROP_DATABASE: {
            const auto& cmd = std::get<DropDatabase>(pr.command);
            return executors::execute_drop_database(cmd, engine, current_db);
        }
        case CommandType::USE: {
            const auto& cmd = std::get<Use>(pr.command);
            return executors::execute_use(cmd, engine, current_db);
        }
        case CommandType::CREATE_TABLE: {
            const auto& cmd = std::get<CreateTable>(pr.command);
            return executors::execute_create_table(cmd, engine, current_db);
        }
        case CommandType::DROP_TABLE: {
            const auto& cmd = std::get<DropTable>(pr.command);
            return executors::execute_drop_table(cmd, engine, current_db);
        }
        case CommandType::INSERT: {
            const auto& cmd = std::get<Insert>(pr.command);
            return executors::execute_insert(cmd, engine, current_db);
        }
        case CommandType::SELECT: {
            const auto& cmd = std::get<Select>(pr.command);
            return executors::execute_select(cmd, engine, current_db);
        }
        case CommandType::UPDATE: {
            const auto& cmd = std::get<Update>(pr.command);
            return executors::execute_update(cmd, engine, current_db);
        }
        case CommandType::DELETE: {
            const auto& cmd = std::get<Delete>(pr.command);
            return executors::execute_delete(cmd, engine, current_db);
        }
        default:
            return {false, "Unsupported command", ""};
        }
    } catch (const std::exception& e) {
        return {false, e.what(), ""};
    }
}
