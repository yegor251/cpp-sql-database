#pragma once
#include "sql/AST.hpp"

namespace sql {
namespace parsers {

ParseResult parse_insert(std::istringstream& iss);

}
}