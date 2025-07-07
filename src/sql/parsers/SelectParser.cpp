#include "sql/parsers/SelectParser.hpp"
#include "sql/parsers/Utils.hpp"
#include <sstream>

namespace sql {
namespace parsers {

ParseResult parse_select(std::istringstream& iss) {
    std::string columns_part;
    char c;
    columns_part.clear();
    while (iss.get(c)) {
        if (toupper(c) == 'F') {
            std::string next;
            next += c;
            for (int i = 0; i < 3 && iss.get(c); ++i) next += c;
            if (to_upper(next) == "FROM") break;
            columns_part += next;
        } else {
            columns_part += c;
        }
    }
    std::string tablename;
    iss >> tablename;
    if (tablename.empty()) return {CommandType::SELECT, {}, false, "No table name"};
    
    std::vector<std::string> columns;
    std::istringstream col_iss(columns_part);
    std::string col;
    while (std::getline(col_iss, col, ',')) {
        col.erase(0, col.find_first_not_of(" \t"));
        col.erase(col.find_last_not_of(" \t") + 1);
        if (!col.empty()) columns.push_back(col);
    }
    if (columns.size() == 1 && columns[0] == "*") columns = {"*"};
    if (columns.empty()) return {CommandType::SELECT, {}, false, "No columns"};
    
    std::vector<std::string> where;
    std::string word;
    iss >> word;
    word = to_upper(word);
    if (word == "WHERE") {
        std::string where_part;
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
    
    return {CommandType::SELECT, Select{columns, tablename, where}, true, ""};
}

} // namespace parsers
} // namespace sql 