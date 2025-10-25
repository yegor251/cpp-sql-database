#include "sql/parsers/Utils.hpp"
#include <algorithm>
#include <cctype>

namespace sql {
namespace parsers {

std::string to_upper(const std::string& s) {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(), ::toupper);
    return r;
}

db::Value parse_value(const std::string& val) {
    std::string clean_val = val;
    if (clean_val.size() >= 2 && clean_val.front() == '"' && clean_val.back() == '"') {
        clean_val = clean_val.substr(1, clean_val.size() - 2);
    }
    
    try {
        int int_val = std::stoi(clean_val);
        return db::Value(int_val);
    } catch (...) {}
    
    try {
        float float_val = std::stof(clean_val);
        return db::Value(float_val);
    } catch (...) {}
    
    std::string upper_val = to_upper(clean_val);
    if (upper_val == "TRUE" || upper_val == "FALSE") {
        return db::Value(upper_val == "TRUE");
    }
    
    if (upper_val == "NULL") {
        return db::Value(db::NullValue{});
    }
    
    return db::Value(clean_val);
}

std::string value_to_string(const db::Value& value) {
    if (std::holds_alternative<int>(value)) {
        return std::to_string(std::get<int>(value));
    } else if (std::holds_alternative<float>(value)) {
        return std::to_string(std::get<float>(value));
    } else if (std::holds_alternative<bool>(value)) {
        return std::get<bool>(value) ? "true" : "false";
    } else if (std::holds_alternative<std::string>(value)) {
        return std::get<std::string>(value);
    } else if (std::holds_alternative<db::NullValue>(value)) {
        return "NULL";
    }
    return "unknown";
}

} // namespace parsers
} // namespace sql 