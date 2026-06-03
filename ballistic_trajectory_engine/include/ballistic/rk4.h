#pragma once
#include "types.h"

namespace ballistic {

Derivative evaluate(const State& s, const MunitionParams& p);
State rk4_step(const State& s, const MunitionParams& p, double dt);

} // namespace ballistic
