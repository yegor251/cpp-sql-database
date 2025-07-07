#include "sql/parsers/CreateParser.hpp"
#include "sql/parsers/Utils.hpp"
#include <sstream>
#include <algorithm>

namespace sql {
namespace parsers {

ParseResult parse_create_database(std::istringstream& iss) {
    std::string dbname;
    iss >> dbname;
    if (dbname.empty()) return {CommandType::CREATE_DATABASE, {}, false, "No database name"};
    return {CommandType::CREATE_DATABASE, CreateDatabase{dbname}, true, ""};
}

ParseResult parse_create_table(std::istringstream& iss) {
    std::string tablename;
    iss >> tablename;
    if (tablename.empty()) return {CommandType::CREATE_TABLE, {}, false, "No table name"};
    
    char c;
    iss >> c;
    if (c != '(') return {CommandType::CREATE_TABLE, {}, false, "Expected '(' after table name"};
    
    std::vector<std::string> columns, types;
    std::string col_name, col_type;
    
    while (iss >> col_name) {
        while (!col_name.empty() && (col_name.back() == ',' || col_name.back() == ';' || col_name.back() == ')')) {
            col_name.pop_back();
        }
        
        iss >> col_type;
        while (!col_type.empty() && (col_type.back() == ',' || col_type.back() == ';' || col_type.back() == ')')) {
            col_type.pop_back();
        }
        
        std::string upper_type = to_upper(col_type);
        if (upper_type != "INT" && upper_type != "FLOAT" && upper_type != "STR" && upper_type != "BOOL") {
            return {CommandType::CREATE_TABLE, {}, false, "Invalid type: " + col_type + ". Supported types: INT, FLOAT, STR, BOOL"};
        }
        
        columns.push_back(col_name);
        types.push_back(upper_type);
        
        iss >> c;
        if (c == ')' || c == ';') break;
        iss.unget();
    }
    
    return {CommandType::CREATE_TABLE, CreateTable{tablename, columns, types}, true, ""};
}

} // namespace parsers
} // namespace sql 