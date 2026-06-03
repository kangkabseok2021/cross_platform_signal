#include "ballistic/rk4.h"
#include <cmath>

namespace ballistic {

Derivative evaluate(const State& s, const MunitionParams& p) {
    double v_mag = std::hypot(s.vx, s.vy);
    double drag  = 0.5 * AIR_DENSITY * p.cd * p.area_m2 / p.mass_kg;
    return {
        s.vx,
        s.vy,
        -drag * v_mag * s.vx,
        -GRAVITY - drag * v_mag * s.vy
    };
}

State rk4_step(const State& s, const MunitionParams& p, double dt) {
    Derivative k1 = evaluate(s, p);
    State s2{s.x + k1.dx*dt/2, s.y + k1.dy*dt/2,
             s.vx + k1.dvx*dt/2, s.vy + k1.dvy*dt/2};
    Derivative k2 = evaluate(s2, p);
    State s3{s.x + k2.dx*dt/2, s.y + k2.dy*dt/2,
             s.vx + k2.dvx*dt/2, s.vy + k2.dvy*dt/2};
    Derivative k3 = evaluate(s3, p);
    State s4{s.x + k3.dx*dt, s.y + k3.dy*dt,
             s.vx + k3.dvx*dt, s.vy + k3.dvy*dt};
    Derivative k4 = evaluate(s4, p);
    return {
        s.x  + dt/6*(k1.dx  + 2*k2.dx  + 2*k3.dx  + k4.dx),
        s.y  + dt/6*(k1.dy  + 2*k2.dy  + 2*k3.dy  + k4.dy),
        s.vx + dt/6*(k1.dvx + 2*k2.dvx + 2*k3.dvx + k4.dvx),
        s.vy + dt/6*(k1.dvy + 2*k2.dvy + 2*k3.dvy + k4.dvy)
    };
}

} // namespace ballistic
