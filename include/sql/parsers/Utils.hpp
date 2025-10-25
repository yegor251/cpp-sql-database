#pragma once
#include <string>
#include "db/Row.hpp"

namespace sql {
namespace parsers {

std::string to_upper(const std::string& s);
db::Value parse_value(const std::string& val);
std::string value_to_string(const db::Value& value);

}
}