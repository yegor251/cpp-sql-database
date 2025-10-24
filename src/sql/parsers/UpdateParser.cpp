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
    std::vector<std::string> where_clauses;
    
    std::string line;
    std::getline(iss, line);
    
    line.erase(0, line.find_first_not_of(" \t"));
    
    if (!line.empty() && line.back() == ';') {
        line.pop_back();
    }
    
    size_t where_pos = line.find(" WHERE ");
    if (where_pos == std::string::npos) {
        where_pos = line.find(" where ");
    }
    
    std::string set_part;
    if (where_pos != std::string::npos) {
        set_part = line.substr(0, where_pos);
        std::string where_part = line.substr(where_pos + 7); // Skip " WHERE "
        
        where_part.erase(0, where_part.find_first_not_of(" \t"));
        where_part.erase(where_part.find_last_not_of(" \t") + 1);
        
        std::istringstream where_stream(where_part);
        std::string condition;
        while (where_stream >> condition) {
            where_clauses.push_back(condition);
        }
    } else {
        set_part = line;
    }
    
    std::istringstream set_stream(set_part);
    std::string assignment;
    
    while (std::getline(set_stream, assignment, ',')) {
        assignment.erase(0, assignment.find_first_not_of(" \t"));
        assignment.erase(assignment.find_last_not_of(" \t") + 1);
        
        if (assignment.empty()) continue;
        
        size_t equals_pos = assignment.find('=');
        if (equals_pos == std::string::npos) {
            return {CommandType::UNKNOWN, {}, false, "Expected = in UPDATE SET clause"};
        }
        
        std::string column_name = assignment.substr(0, equals_pos);
        std::string value = assignment.substr(equals_pos + 1);
        
        column_name.erase(0, column_name.find_first_not_of(" \t"));
        column_name.erase(column_name.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        
        if (column_name.empty() || value.empty()) {
            return {CommandType::UNKNOWN, {}, false, "Empty column name or value in UPDATE SET clause"};
        }
        
        set_clauses.push_back(column_name + "=" + value);
    }
    
    if (set_clauses.empty()) {
        return {CommandType::UNKNOWN, {}, false, "No columns specified for update"};
    }
    
    return {CommandType::UPDATE, Update{table_name, set_clauses, where_clauses}, true, ""};
}

}