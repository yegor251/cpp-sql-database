#pragma once
#include "sql/AST.hpp"

namespace sql::parsers {
    ParseResult parse_update(std::istringstream& iss);
}
