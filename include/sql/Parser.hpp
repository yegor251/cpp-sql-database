#pragma once
#include <string>
#include "sql/AST.hpp"

namespace sql {

class Parser {
public:
    static ParseResult parse(const std::string& query);
};

}
