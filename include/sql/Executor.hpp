#pragma once
#include "sql/AST.hpp"
#include "db/StorageEngine.hpp"

namespace sql {

struct ExecResult {
    bool ok;
    std::string error;
    std::string result;
};

class Executor {
public:
    static ExecResult execute(const ParseResult& pr, db::StorageEngine& engine);
    static std::string current_db;
};

}
