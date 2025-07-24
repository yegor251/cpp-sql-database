#include "db/Row.hpp"

namespace db {

void to_json(json& j, const Row& r) {
    j = json::array();
    for (const auto& value : r.values_) {
        std::visit([&j](const auto& v) {
            j.push_back(v);
        }, value);
    }
}

void from_json(const json& j, Row& r) {
    r.values_.clear();
    for (const auto& item : j) {
        if (item.is_number_integer()) {
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
