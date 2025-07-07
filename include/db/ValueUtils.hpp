#pragma once
#include "db/Row.hpp"
#include <string>

namespace db {

// Утилиты для работы с Value
inline std::string value_to_string(const Value& v) {
    return std::visit([](const auto& val) -> std::string {
        if constexpr (std::is_same_v<decltype(val), bool>) {
            return val ? "true" : "false";
        } else {
            return std::to_string(val);
        }
    }, v);
}

inline bool value_equals(const Value& a, const Value& b) {
    return std::visit([](const auto& va, const auto& vb) -> bool {
        if constexpr (std::is_same_v<decltype(va), decltype(vb)>) {
            return va == vb;
        } else {
            return false; // Разные типы
        }
    }, a, b);
}

inline bool value_less(const Value& a, const Value& b) {
    return std::visit([](const auto& va, const auto& vb) -> bool {
        if constexpr (std::is_same_v<decltype(va), decltype(vb)>) {
            return va < vb;
        } else {
            return false; // Разные типы
        }
    }, a, b);
}

} // namespace db 