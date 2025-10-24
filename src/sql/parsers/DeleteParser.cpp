#include "sql/parsers/DeleteParser.hpp"
#include "sql/parsers/Utils.hpp"
#include <string>
#include <sstream>
#include <cctype>

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
    
    if (!tablename.empty() && tablename.back() == ';') {
        tablename.pop_back();
    }
    
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
        
        std::string trimmed_where = where_part;
        while (!trimmed_where.empty() && std::isspace(trimmed_where.front())) {
            trimmed_where.erase(trimmed_where.begin());
        }
        while (!trimmed_where.empty() && std::isspace(trimmed_where.back())) {
            trimmed_where.pop_back();
        }
        
        if (!trimmed_where.empty()) {
            std::istringstream where_stream(trimmed_where);
            std::string condition;
            while (where_stream >> condition) {
                where.push_back(condition);
            }
        }
    }
    
    return {CommandType::DELETE, Delete{tablename, where}, true, ""};
}

}
}