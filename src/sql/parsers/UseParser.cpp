#include "sql/parsers/UseParser.hpp"
#include "sql/parsers/Utils.hpp"
#include <sstream>

namespace sql {
namespace parsers {

ParseResult parse_use(std::istringstream& iss) {
    std::string dbname;
    iss >> dbname;
    if (dbname.empty()) return {CommandType::USE, {}, false, "No database name"};
    return {CommandType::USE, Use{dbname}, true, ""};
}

} // namespace parsers
} // namespace sql 