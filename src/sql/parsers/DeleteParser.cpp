#include "sql/parsers/DeleteParser.hpp"
#include "sql/parsers/Utils.hpp"
#include <sstream>

namespace sql {
namespace parsers {

ParseResult parse_delete(std::istringstream& iss) {
    std::string word;
    iss >> word;
    word = to_upper(word);
    if (word != "FROM") return {CommandType::DELETE, {}, false, "Expected FROM"};
    
    std::string tablename;
    iss >> tablename;
    if (tablename.empty()) return {CommandType::DELETE, {}, false, "No table name"};
    
    std::vector<std::string> where;
    iss >> word;
    word = to_upper(word);
    if (word == "WHERE") {
        std::string where_part;
        char c;
        while (iss.get(c)) {
            if (c == ';') break;
            where_part += c;
        }
        std::istringstream where_iss(where_part);
        std::string token;
        while (where_iss >> token) {
            where.push_back(token);
        }
    }
    
    return {CommandType::DELETE, Delete{tablename, where}, true, ""};
}

} // namespace parsers
} // namespace sql 