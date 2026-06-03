#include <ballistic/engine.h>
#include <cstdlib>
#include <iostream>

int main() {
    ballistic::BallisticEngine e;
    auto r = e.compute(45.0, 400.0, ballistic::Artillery155mm{});
    if (!r.has_value() || r->empty()) {
        std::cerr << "test_package FAILED: no trajectory returned\n";
        return EXIT_FAILURE;
    }
    std::cout << "test_package OK: " << r->size() << " trajectory points, "
              << "range = " << r->back().x_m << " m\n";
    return EXIT_SUCCESS;
}
