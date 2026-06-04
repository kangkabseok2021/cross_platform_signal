#include "heat_solver.h"
#include <Eigen/Dense>
#include <algorithm>
#include <cmath>
#include <stdexcept>

void HeatSolver::solve(const std::vector<double>& kappa,
                       const std::vector<double>& q_source,
                       const Params& p)
{
    nx_ = p.nx; ny_ = p.ny;
    const int N = nx_ * ny_;

    if (static_cast<int>(kappa.size()) != N ||
        static_cast<int>(q_source.size()) != N)
        throw std::invalid_argument("kappa/q_source size mismatch");

    // Initialise temperature field with boundary conditions
    T_.assign(static_cast<size_t>(N), 0.0);
    const double dx = 1.0;   // normalised grid spacing

    // Set left/right Dirichlet BCs column-wise
    for (int j = 0; j < ny_; ++j) {
        T_[static_cast<size_t>(j * nx_)]            = p.bc_left;
        T_[static_cast<size_t>(j * nx_ + nx_ - 1)] = p.bc_right;
    }

    Eigen::Map<Eigen::MatrixXd> T_mat(T_.data(), nx_, ny_);
    const Eigen::Map<const Eigen::MatrixXd> kap(kappa.data(), nx_, ny_);
    const Eigen::Map<const Eigen::MatrixXd> q(q_source.data(), nx_, ny_);

    // Explicit Euler FDM: T_new = T + dt * (Laplacian(T) * kappa + q)
    for (int step = 0; step < p.n_steps; ++step) {
        Eigen::MatrixXd lap = Eigen::MatrixXd::Zero(nx_, ny_);

        for (int j = 1; j < ny_ - 1; ++j) {
            for (int i = 1; i < nx_ - 1; ++i) {
                double d2Tdx = (T_mat(i+1,j) - 2*T_mat(i,j) + T_mat(i-1,j)) / (dx*dx);
                double d2Tdy = (T_mat(i,j+1) - 2*T_mat(i,j) + T_mat(i,j-1)) / (dx*dx);
                lap(i,j) = kap(i,j) * (d2Tdx + d2Tdy) + q(i,j);
            }
        }
        T_mat += p.dt * lap;

        // Re-enforce Dirichlet BCs each step
        for (int j = 0; j < ny_; ++j) {
            T_mat(0,     j) = p.bc_left;
            T_mat(nx_-1, j) = p.bc_right;
        }
    }

    t_min_ = *std::min_element(T_.begin(), T_.end());
    t_max_ = *std::max_element(T_.begin(), T_.end());
}
