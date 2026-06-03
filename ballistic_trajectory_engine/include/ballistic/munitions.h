#pragma once
#include <variant>

namespace ballistic {

struct Artillery155mm {
    static constexpr double mass_kg = 43.5;
    static constexpr double cd      = 0.295;
    static constexpr double area_m2 = 0.018869; // pi*(0.155/2)^2
};

struct Mortar81mm {
    static constexpr double mass_kg = 4.1;
    static constexpr double cd      = 0.42;
    static constexpr double area_m2 = 0.005153; // pi*(0.081/2)^2
};

struct APFSDS120mm {
    static constexpr double mass_kg = 4.6;
    static constexpr double cd      = 0.12;
    static constexpr double area_m2 = 0.011310; // pi*(0.120/2)^2
};

using MunitionType = std::variant<Artillery155mm, Mortar81mm, APFSDS120mm>;

} // namespace ballistic
