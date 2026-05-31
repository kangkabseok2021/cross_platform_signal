#pragma once
#include <concepts>
#include <string_view>

/* Compile-time duck-typing via C++20 Concept — no inheritance required */
template<typename T>
concept AcousticModelConcept = requires(const T& m, float Lw, float r) {
    { m.computeSPL(Lw, r) } -> std::same_as<float>;
    { m.name()             } -> std::convertible_to<std::string_view>;
};

/* Runtime polymorphism via virtual base — used by AcousticEngine (Strategy) */
class IAcousticModel {
public:
    virtual ~IAcousticModel() = default;
    [[nodiscard]] virtual float           computeSPL(float Lw, float r) const = 0;
    [[nodiscard]] virtual std::string_view name() const noexcept = 0;
};
