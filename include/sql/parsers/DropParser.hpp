#pragma once
#include "sql/AST.hpp"

namespace sql {
namespace parsers {

ParseResult parse_drop_database(std::istringstream& iss);
ParseResult parse_drop_table(std::istringstream& iss);

} // namespace parsers
} // namespace sql 