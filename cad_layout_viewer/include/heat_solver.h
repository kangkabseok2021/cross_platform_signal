#pragma once
#include <QRectF>
#include <vector>

// 2D explicit FDM heat solver on a regular nx×ny grid.
// Discretises:  ∂T/∂t = ∇·(κ ∇T) + q
// Time-stepping: explicit Euler (stable when dt ≤ dx²/(4·α_max)).
class HeatSolver {
public:
    struct Params {
        int    nx{64};
        int    ny{64};
        double dt{1e-3};        // time step (s)
        int    n_steps{200};    // iteration count
        double bc_left{100.0};  // Dirichlet left  boundary (°C)
        double bc_right{0.0};   // Dirichlet right boundary (°C)
    };

    void solve(const std::vector<double>& kappa,   // nx*ny thermal conductivity
               const std::vector<double>& q_source, // nx*ny heat source (W/m³)
               const Params& p);

    [[nodiscard]] const std::vector<double>& temperature() const noexcept { return T_; }
    [[nodiscard]] int nx() const noexcept { return nx_; }
    [[nodiscard]] int ny() const noexcept { return ny_; }
    [[nodiscard]] double tMin() const noexcept { return t_min_; }
    [[nodiscard]] double tMax() const noexcept { return t_max_; }

private:
    std::vector<double> T_;
    int nx_{0}, ny_{0};
    double t_min_{0.0}, t_max_{0.0};
};
