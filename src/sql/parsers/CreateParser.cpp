#include "sql/parsers/CreateParser.hpp"
#include "sql/parsers/Utils.hpp"
#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>

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
    
    if (!tablename.empty() && tablename.back() == ';') {
        tablename.pop_back();
    }
    
    char c;
    iss >> c;
    if (c != '(') return {CommandType::CREATE_TABLE, {}, false, "Expected '(' after table name"};
    
    std::string table_def;
    int paren_depth = 1;
    while (iss.get(c) && paren_depth > 0) {
        if (c == '(') {
            paren_depth++;
            table_def += c;
        } else if (c == ')') {
            paren_depth--;
            if (paren_depth > 0) {
                table_def += c;
            }
        } else {
            table_def += c;
        }
    }
    
    if (!table_def.empty() && table_def.back() == ';') {
        table_def.pop_back();
    }
    
    table_def.erase(table_def.find_last_not_of(" \t") + 1);
    
    std::vector<std::string> columns, types;
    std::vector<ForeignKeyConstraint> foreign_keys;
    
    std::vector<std::string> column_definitions;
    std::string current_def;
    bool in_parens = false;
    
    for (char ch : table_def) {
        if (ch == '(') {
            in_parens = true;
            current_def += ch;
        } else if (ch == ')') {
            in_parens = false;
            current_def += ch;
        } else if (ch == ',' && !in_parens) {
            if (!current_def.empty()) {
                column_definitions.push_back(current_def);
                current_def.clear();
            }
        } else {
            current_def += ch;
        }
    }
    
    if (!current_def.empty()) {
        column_definitions.push_back(current_def);
    }
    
    for (size_t def_idx = 0; def_idx < column_definitions.size(); ++def_idx) {
        const auto& def = column_definitions[def_idx];
        
        std::string trimmed_def = def;
        trimmed_def.erase(0, trimmed_def.find_first_not_of(" \t"));
        trimmed_def.erase(trimmed_def.find_last_not_of(" \t") + 1);
        
        if (trimmed_def.empty()) {
            continue;
        }
        
        std::istringstream def_stream(trimmed_def);
        std::vector<std::string> tokens;
        std::string token;
        
        while (def_stream >> token) {
            tokens.push_back(token);
        }
        
        if (tokens.empty()) {
            continue;
        }
        
        bool has_fk = false;
        size_t fk_index = 0;
        for (size_t i = 0; i < tokens.size(); ++i) {
            if (to_upper(tokens[i]) == "FK") {
                has_fk = true;
                fk_index = i;
                break;
            }
        }
        
        if (has_fk) {

            std::string col_name, col_type;
            if (fk_index >= 2) {
                col_name = tokens[0];
                col_type = to_upper(tokens[1]);
                
                if (col_type != "INT" && col_type != "FLOAT" && col_type != "STR" && col_type != "BOOL") {
                    return {CommandType::CREATE_TABLE, {}, false, "Invalid type: " + tokens[1] + ". Supported types: INT, FLOAT, STR, BOOL"};
                }
                
                columns.push_back(col_name);
                types.push_back(col_type);
            }
            
            if (tokens.size() < fk_index + 2) {
                return {CommandType::CREATE_TABLE, {}, false, "Invalid FK syntax - not enough tokens"};
            }
            
            std::string ref_part = tokens[fk_index + 1];
            
            std::string fk_column = col_name;
            
            size_t paren_pos = ref_part.find('(');
            std::string ref_table = ref_part.substr(0, paren_pos);
            std::string ref_column = ref_part.substr(paren_pos + 1);
            ref_column.pop_back();
            
            foreign_keys.emplace_back(fk_column, ref_table, ref_column);
            continue;
        }
        
        if (tokens.size() != 2) {
            return {CommandType::CREATE_TABLE, {}, false, "Invalid column definition: " + trimmed_def};
        }
        
        std::string col_name = tokens[0];
        std::string col_type = to_upper(tokens[1]);
        
        if (col_type != "INT" && col_type != "FLOAT" && col_type != "STR" && col_type != "BOOL") {
            return {CommandType::CREATE_TABLE, {}, false, "Invalid type: " + tokens[1] + ". Supported types: INT, FLOAT, STR, BOOL"};
        }
        
        columns.push_back(col_name);
        types.push_back(col_type);
    }
    
    return {CommandType::CREATE_TABLE, CreateTable{tablename, columns, types, {}, foreign_keys, {}}, true, ""};
}

}
}