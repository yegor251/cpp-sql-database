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

void to_json(json& j, const Row& r);
void from_json(const json& j, Row& r);

} // namespace db