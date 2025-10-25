#include "sql/parsers/DropParser.hpp"
#include <string>
#include <sstream>

namespace sql {
namespace parsers {

ParseResult parse_drop_database(std::istringstream& iss) {
    std::vector<std::string> dbnames;
    std::string dbname;
    while (iss >> dbname) {
        if (!dbname.empty()) dbnames.push_back(dbname);
    }
    if (dbnames.empty()) return {CommandType::DROP_DATABASE, {}, false, "No database name"};
    return {CommandType::DROP_DATABASE, DropDatabase{dbnames}, true, ""};
}

ParseResult parse_drop_table(std::istringstream& iss) {
    std::vector<std::string> tablenames;
    std::string tablename;
    while (iss >> tablename) {
        if (!tablename.empty()) tablenames.push_back(tablename);
    }
    if (tablenames.empty()) return {CommandType::DROP_TABLE, {}, false, "No table name"};
    return {CommandType::DROP_TABLE, DropTable{tablenames}, true, ""};
}

}
}