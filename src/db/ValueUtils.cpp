#include "db/ValueUtils.hpp"

namespace db {

std::string value_to_string(const Value& v) {
    return std::visit([](const auto& val) -> std::string {
        if constexpr (std::is_same_v<decltype(val), bool>) {
            return val ? "true" : "false";
        } else if constexpr (std::is_same_v<decltype(val), std::string>) {
            return val;
        } else if constexpr (std::is_same_v<decltype(val), int> || std::is_same_v<decltype(val), float>) {
            return std::to_string(val);
        } else {
            return "unknown";
        }
    }, v);
}

bool value_equals(const Value& a, const Value& b) noexcept {
    return std::visit([](const auto& va, const auto& vb) -> bool {
        if constexpr (std::is_same_v<decltype(va), decltype(vb)>) {
            return va == vb;
        } else {
            return false;
        }
    }, a, b);
}

bool value_less(const Value& a, const Value& b) noexcept {
    return std::visit([](const auto& va, const auto& vb) -> bool {
        if constexpr (std::is_same_v<decltype(va), decltype(vb)>) {
            return va < vb;
        } else {
            return false;
        }
    }, a, b);
}

} // namespace db 