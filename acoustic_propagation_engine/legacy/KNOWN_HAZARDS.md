# Legacy C Acoustic Module — Known Hazards

Catalogue of defects deliberately preserved as the baseline before the C++20 refactor.

| # | File | Line | Hazard | C++20 fix |
|---|---|---|---|---|
| 1 | `acoustic.h` | 16 | `acoustic_alloc_grid` returns a raw `float*` — caller must call `acoustic_free_grid`. Failure to do so leaks memory (demonstrated by `demo_leak.c`). | `AcousticEngine::computeGrid` returns `std::vector<float>` (RAII, freed automatically). |
| 2 | `acoustic.c` | 6 | `static float g_air_absorption` is a mutable file-scope global. Two concurrent calls with different atmospheric conditions corrupt each other's state (non-reentrant). | `inline constexpr float kAirAbsorption_1kHz = 0.004f` — compile-time constant, no global state. |
| 3 | `acoustic.c` | 22 | `x_coords` and `y_coords` are `float*` with no length information. Passing `nx`/`ny` larger than the arrays causes a buffer overrun. | `std::span<const float>` carries both pointer and length — bounds violation is caught by `span::operator[]` in debug mode. |
| 4 | `acoustic.c` | 28 | When `r = 0`, `log10f(0) = -Inf`, producing `Lp = +Inf`. No guard exists. Callers who pass `src_x == x_coords[i]` receive a silent infinity. | `const float r_safe = std::max(r, kMinDistance)` — zero-distance singularity eliminated by construction. |

## Evidence

```bash
# Hazard #1 — memory leak
valgrind --leak-check=full ./demo_leak
# Expected: LEAK SUMMARY: definitely lost: 3 blocks

# Physics correctness despite hazards #2–4
./demo_safe > /tmp/c_out.txt
python3 ../scripts/validate_c.py /tmp/c_out.txt
# Expected: PASS — max error < 0.001 dB

# Hazard #4 — confirmed by GoogleTest ZeroRadiusGuard.NoNaN_OrInf
# which calls InverseSquareLawModel.computeSPL(80, 0) and asserts isfinite()
```
