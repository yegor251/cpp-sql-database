#pragma once
#include "db/Row.hpp"
#include <string>

namespace db {

std::string value_to_string(const Value& v);
bool value_equals(const Value& a, const Value& b) noexcept;
bool value_less(const Value& a, const Value& b) noexcept;

} // namespace db 