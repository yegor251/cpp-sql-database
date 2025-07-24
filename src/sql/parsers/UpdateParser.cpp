#include "sql/parsers/UpdateParser.hpp"
#include "sql/parsers/Utils.hpp"
#include <string>
#include <sstream>
#include <vector>

namespace sql::parsers {

ParseResult parse_update(std::istringstream& iss) {
    std::string table_name;
    iss >> table_name;
    
    if (table_name.empty()) {
        return {CommandType::UNKNOWN, {}, false, "Missing table name in UPDATE statement"};
    }
    
    std::string set_keyword;
    iss >> set_keyword;
    set_keyword = to_upper(set_keyword);
    
    if (set_keyword != "SET") {
        return {CommandType::UNKNOWN, {}, false, "Expected SET keyword in UPDATE statement"};
    }
    
    std::vector<std::string> set_clauses;
    std::string column_name, equals_sign, value;
    
    while (iss >> column_name >> equals_sign >> value) {
        if (equals_sign != "=") {
            return {CommandType::UNKNOWN, {}, false, "Expected = in UPDATE SET clause"};
        }
        
        set_clauses.push_back(column_name + "=" + value);
        
        // Check for comma or end
        char next_char = iss.peek();
        if (next_char == ',') {
            iss.get(); // consume comma
            iss >> std::ws; // skip whitespace
        } else if (next_char == ';' || next_char == '\n' || next_char == EOF) {
            break;
        }
    }
    
    if (set_clauses.empty()) {
        return {CommandType::UNKNOWN, {}, false, "No columns specified for update"};
    }
    
    return {CommandType::UPDATE, Update{table_name, set_clauses}, true, ""};
}

} // namespace sql::parsers 