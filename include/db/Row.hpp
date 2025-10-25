#pragma once
#include <vector>
#include <string>
#include <variant>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace db {

struct NullValue {
    bool operator==(const NullValue&) const { return true; }
    bool operator<(const NullValue&) const { return false; }
};
using Value = std::variant<int, float, std::string, bool, NullValue>;

class Row {
public:
    Row() = default;
    explicit Row(std::vector<Value> values) : values_(std::move(values)) {}

    const std::vector<Value>& get_values() const noexcept { return values_; }
    std::vector<Value>& get_values() noexcept { return values_; }

    friend void to_json(json& j, const Row& r);
    friend void from_json(const json& j, Row& r);

private:
    std::vector<Value> values_;
};

void to_json(json& j, const Row& r);
void from_json(const json& j, Row& r);

} // namespace db