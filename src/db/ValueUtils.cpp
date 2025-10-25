#include "db/ValueUtils.hpp"

namespace db {

std::string value_to_string(const Value& v) {
    if (std::holds_alternative<int>(v)) {
        int val = std::get<int>(v);
        return std::to_string(val);
    } else if (std::holds_alternative<float>(v)) {
        float val = std::get<float>(v);
        return std::to_string(val);
    } else if (std::holds_alternative<bool>(v)) {
        bool val = std::get<bool>(v);
        return val ? "true" : "false";
    } else if (std::holds_alternative<std::string>(v)) {
        std::string val = std::get<std::string>(v);
        return val;
    } else if (std::holds_alternative<NullValue>(v)) {
        return "NULL";
    } else {
        return "unknown";
    }
}

bool value_equals(const Value& a, const Value& b) noexcept {
    if (std::holds_alternative<NullValue>(a) && std::holds_alternative<NullValue>(b)) {
        return true;
    }
    if (std::holds_alternative<NullValue>(a) || std::holds_alternative<NullValue>(b)) {
        return false;
    }
    
    return std::visit([](const auto& va, const auto& vb) -> bool {
        if constexpr (std::is_same_v<decltype(va), decltype(vb)>) {
            return va == vb;
        } else {
            return false;
        }
    }, a, b);
}

bool value_less(const Value& a, const Value& b) noexcept {
    if (std::holds_alternative<NullValue>(a) || std::holds_alternative<NullValue>(b)) {
        return false;
    }
    
    return std::visit([](const auto& va, const auto& vb) -> bool {
        if constexpr (std::is_same_v<decltype(va), decltype(vb)>) {
            return va < vb;
        } else {
            return false;
        }
    }, a, b);
}

}