#pragma once
#include "sql/AST.hpp"

namespace sql {
namespace parsers {

ParseResult parse_select(std::istringstream& iss);

}
}