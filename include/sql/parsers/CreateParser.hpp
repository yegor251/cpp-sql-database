#pragma once
#include "sql/AST.hpp"

namespace sql {
namespace parsers {

ParseResult parse_create_database(std::istringstream& iss);
ParseResult parse_create_table(std::istringstream& iss);

} // namespace parsers
} // namespace sql 