# Ballistic Physics Model

## Equations of Motion

The projectile's trajectory is governed by Newton's second law with gravity and aerodynamic drag:

```
d²x/dt² = -(ρ·Cd·A / 2m) · |v| · vx
d²y/dt² = -g - (ρ·Cd·A / 2m) · |v| · vy
```

| Symbol | Value | Description |
|--------|-------|-------------|
| g      | 9.80665 m/s² | Standard gravity |
| ρ      | 1.225 kg/m³  | Air density at sea level (ISA) |
| Cd     | munition-specific | Drag coefficient |
| A      | munition-specific | Reference cross-sectional area |
| m      | munition-specific | Projectile mass |

## 4th-Order Runge-Kutta Integration

The state vector is **S = (x, y, vx, vy)**. At each time step `dt`:

```
k1 = f(S)
k2 = f(S + dt/2 · k1)
k3 = f(S + dt/2 · k2)
k4 = f(S + dt  · k3)
S_next = S + dt/6 · (k1 + 2k2 + 2k3 + k4)
```

Default `dt = 0.01 s` gives < 0.1 m range error for typical munitions.

## Munition Parameters

| Munition       | Mass (kg) | Cd    | Area (m²) | Typical muzzle velocity |
|----------------|-----------|-------|-----------|------------------------|
| Artillery 155mm M107 HE | 43.5 | 0.295 | 0.01887 | 560–850 m/s |
| Mortar 81mm M821 HE     | 4.1  | 0.42  | 0.00515 | 150–320 m/s |
| APFSDS 120mm (kinetic)  | 4.6  | 0.12  | 0.01131 | 1600–1750 m/s |

## Drag Model Limitations

- Assumes **constant air density** (sea-level ISA). No altitude correction.
- Drag coefficient **Cd is constant** — no Mach-number dependency.
- No wind, Coriolis effect, or spin stabilisation modelled.
- Landing point found by linear interpolation when y crosses zero.

## Validation

The `NoDragMatchesAnalyticRange` test verifies: when Cd = 0,
range = v₀² sin(2θ) / g (within 1 m for v₀=100 m/s, θ=45°).

The `MaxAltitudeMatchesAnalytic` test verifies: vertical shot ymax = v₀²/(2g) (within 0.5 m).
