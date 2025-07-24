#include "sql/parsers/InsertParser.hpp"
#include "sql/parsers/Utils.hpp"
#include <string>
#include <sstream>

namespace sql {
namespace parsers {

ParseResult parse_insert(std::istringstream& iss) {
    std::string word;
    iss >> word;
    word = to_upper(word);
    if (word != "INTO") return {CommandType::INSERT, {}, false, "Expected INTO"};
    
    std::string tablename;
    iss >> tablename;
    if (tablename.empty()) return {CommandType::INSERT, {}, false, "No table name"};
    
    char c;
    iss >> c;
    std::vector<std::string> columns;
    if (c == '(') {
        std::string col;
        while (iss >> col) {
            if (col.back() == ',') col.pop_back();
            if (col.back() == ')') {
                col.pop_back();
                columns.push_back(col);
                break;
            }
            columns.push_back(col);
        }
    } else {
        iss.unget();
    }
    
    iss >> word;
    word = to_upper(word);
    if (word != "VALUES") return {CommandType::INSERT, {}, false, "Expected VALUES"};
    
    iss >> c;
    if (c != '(') return {CommandType::INSERT, {}, false, "Expected '(' after VALUES"};
    
    std::vector<db::Value> values;
    std::string val;
    while (iss >> val) {
        while (!val.empty() && (val.back() == ',' || val.back() == ';' || val.back() == ')')) {
            val.pop_back();
        }
        values.push_back(parse_value(val));
        char peek = iss.peek();
        if (peek == ')' || peek == ';') {
            iss.get();
            break;
        }
    }
    
    return {CommandType::INSERT, Insert{tablename, columns, values}, true, ""};
}

} // namespace parsers
} // namespace sql 