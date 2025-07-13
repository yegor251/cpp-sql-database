#pragma once
#include "db/Row.hpp"
#include <string>

namespace db {

// Утилиты для работы с Value
std::string value_to_string(const Value& v);
bool value_equals(const Value& a, const Value& b);
bool value_less(const Value& a, const Value& b);

} // namespace db 