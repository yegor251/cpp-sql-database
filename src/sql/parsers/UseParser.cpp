#include "sql/parsers/UseParser.hpp"
#include "sql/parsers/Utils.hpp"
#include <string>
#include <sstream>

namespace sql {
namespace parsers {

ParseResult parse_use(std::istringstream& iss) {
    std::string dbname;
    while (iss >> dbname) {
        if (dbname.empty()) return {CommandType::USE, {}, false, "No database name"};
        return {CommandType::USE, Use{dbname}, true, ""};
    }
    return {CommandType::USE, {}, false, "No database name"};
}

} // namespace parsers
} // namespace sql 