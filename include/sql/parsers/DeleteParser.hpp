#pragma once
#include "sql/AST.hpp"

namespace sql {
namespace parsers {

ParseResult parse_delete(std::istringstream& iss);

} // namespace parsers
} // namespace sql