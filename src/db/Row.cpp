#include "db/Row.hpp"

namespace db {

void to_json(json& j, const Row& r) {
    j = json::array();
    for (const auto& value : r.values_) {
        if (std::holds_alternative<NullValue>(value)) {
            j.push_back(nullptr);
        } else if (std::holds_alternative<int>(value)) {
            j.push_back(std::get<int>(value));
        } else if (std::holds_alternative<float>(value)) {
            j.push_back(std::get<float>(value));
        } else if (std::holds_alternative<bool>(value)) {
            j.push_back(std::get<bool>(value));
        } else if (std::holds_alternative<std::string>(value)) {
            j.push_back(std::get<std::string>(value));
        }
    }
}

void from_json(const json& j, Row& r) {
    r.values_.clear();
    for (const auto& item : j) {
        if (item.is_null()) {
            r.values_.emplace_back(NullValue{});
        } else if (item.is_number_integer()) {
            r.values_.emplace_back(item.get<int>());
        } else if (item.is_number_float()) {
            r.values_.emplace_back(item.get<float>());
        } else if (item.is_boolean()) {
            r.values_.emplace_back(item.get<bool>());
        } else {
            r.values_.emplace_back(item.get<std::string>());
        }
    }
}

} // namespace db
