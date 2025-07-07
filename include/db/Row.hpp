#pragma once
#include <vector>
#include <string>
#include <variant>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace db {

using Value = std::variant<int, float, std::string, bool>;

struct Row {
    std::vector<Value> values;

    friend void to_json(json& j, const Row& r);
    friend void from_json(const json& j, Row& r);
};

inline void to_json(json& j, const Row& r) {
    j = json::array();
    for (const auto& value : r.values) {
        std::visit([&j](const auto& v) {
            j.push_back(v);
        }, value);
    }
}

inline void from_json(const json& j, Row& r) {
    r.values.clear();
    for (const auto& item : j) {
        if (item.is_number_integer()) {
            r.values.emplace_back(item.get<int>());
        } else if (item.is_number_float()) {
            r.values.emplace_back(item.get<float>());
        } else if (item.is_boolean()) {
            r.values.emplace_back(item.get<bool>());
        } else {
            r.values.emplace_back(item.get<std::string>());
        }
    }
}

} // namespace db