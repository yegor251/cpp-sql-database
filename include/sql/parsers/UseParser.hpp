#pragma once
#include "sql/AST.hpp"

namespace sql {
namespace parsers {

ParseResult parse_use(std::istringstream& iss);

}
}