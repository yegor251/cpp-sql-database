#include "sql/Parser.hpp"
#include "sql/parsers/CreateParser.hpp"
#include "sql/parsers/DropParser.hpp"
#include "sql/parsers/InsertParser.hpp"
#include "sql/parsers/SelectParser.hpp"
#include "sql/parsers/UpdateParser.hpp"
#include "sql/parsers/UseParser.hpp"
#include "sql/parsers/DeleteParser.hpp"
#include "sql/parsers/OtherParsers.hpp"
#include "sql/parsers/Utils.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>

using namespace sql;

static std::string to_upper(const std::string& s) {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(), ::toupper);
    return r;
}

// Добавляем функцию для конвертации строки в Value
static db::Value parse_value(const std::string& val) {
    // Убираем кавычки если есть
    std::string clean_val = val;
    if (clean_val.size() >= 2 && clean_val.front() == '"' && clean_val.back() == '"') {
        clean_val = clean_val.substr(1, clean_val.size() - 2);
    }
    
    // Попытка парсинга как int
    try {
        int int_val = std::stoi(clean_val);
        return db::Value(int_val);
    } catch (...) {}
    
    // Попытка парсинга как float
    try {
        float float_val = std::stof(clean_val);
        return db::Value(float_val);
    } catch (...) {}
    
    // Проверка на bool
    std::string upper_val = to_upper(clean_val);
    if (upper_val == "TRUE" || upper_val == "FALSE") {
        return db::Value(upper_val == "TRUE");
    }
    
    // По умолчанию строка
    return db::Value(clean_val);
}

ParseResult Parser::parse(const std::string& query) {
    std::istringstream iss(query);
    std::string word;
    iss >> word;
    word = parsers::to_upper(word);

    if (word == "CREATE") {
        iss >> word;
        word = parsers::to_upper(word);
        if (word == "DATABASE") {
            return parsers::parse_create_database(iss);
        }
        if (word == "TABLE") {
            return parsers::parse_create_table(iss);
        }
    }
    
    if (word == "DROP") {
        iss >> word;
        word = parsers::to_upper(word);
        if (word == "DATABASE") {
            return parsers::parse_drop_database(iss);
        }
        if (word == "TABLE") {
            return parsers::parse_drop_table(iss);
        }
    }
    
    if (word == "INSERT") {
        return parsers::parse_insert(iss);
    }
    
    if (word == "SELECT") {
        return parsers::parse_select(iss);
    }
    
    if (word == "UPDATE") {
        return parsers::parse_update(iss);
    }
    
    if (word == "DELETE") {
        return parsers::parse_delete(iss);
    }
    
    if (word == "USE") {
        return parsers::parse_use(iss);
    }
    
    return {CommandType::UNKNOWN, {}, false, "Unknown or unsupported command"};
}
