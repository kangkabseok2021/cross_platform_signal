# High-Precision Optical Positioning Simulator

A C++17 simulation of micro-level positioning software for eyeglass lens grinding. A numerical engine transforms aspheric surface models into sub-micrometre toolpaths using cubic spline interpolation solved via the Thomas algorithm. Four simulated HAL devices are driven through a six-state manufacturing FSM. A lock-free SPSC ring buffer decouples the 10 ms control loop from asynchronous CSV telemetry logging.

---

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│  LensSurface                                                    │
│  radius R, conic K, 6 Zernike coefficients aᵢ                  │
│  z(r) = r²/(R·(1+√(1-(1+K)r²/R²))) + Σ aᵢ·(r/R)^(2i+2)       │
└──────────────────────┬──────────────────────────────────────────┘
                       │ sag(r) in mm
┌──────────────────────▼──────────────────────────────────────────┐
│  ToolpathGenerator                                              │
│  Polar raster: r = 0, h, 2h … r_max  ×  n_angles               │
│  Natural cubic spline: Thomas O(N) per radial profile           │
│  evalAt(r_mm) → sub-sample sag in μm                           │
└──────────────────────┬──────────────────────────────────────────┘
                       │ ToolpathPoint {r_mm, theta_rad, z_um}
┌──────────────────────▼──────────────────────────────────────────┐
│  PositioningFsm                                                 │
│  IDLE → INITIALISING → CALIBRATING → READY → ACTIVE → FAULT    │
│  calls init() / selfTest() / shutdown() on IDevice vector       │
└──────────┬──────────────────────────────────────────────────────┘
           │ state == ACTIVE: 10 ms control loop
┌──────────▼──────────────────────────────────────────────────────┐
│  HAL Layer  (IDevice abstract)                                  │
│  SpindleMotor   — linear ramp to target RPM                     │
│  LinearAxis     — X/Y/Z micro-step positioning                  │
│  PressureSensor — contact force + Gaussian noise                │
│  SafetyInterlock— watchdog; trips if ping() > max_interval_ms   │
└──────────┬──────────────────────────────────────────────────────┘
           │ Frame {ts_us, x_um, y_um, z_um, rpm, deviation_um}
┌──────────▼──────────────────────────────────────────────────────┐
│  TelemetryLogger  (SPSC lock-free ring buffer)                  │
│  push() — producer, never blocks                                │
│  writerLoop() — consumer thread, drains buffer → CSV every 10ms │
└─────────────────────────────────────────────────────────────────┘
```

---

## Lens Surface Model

```
z(r) = r² / (R·(1 + √(1 − (1+K)·r²/R²)))  +  Σᵢ aᵢ·(r/R)^(2i+2)
```

| Parameter | Symbol | Description |
|---|---|---|
| Base radius | R | Radius of curvature (mm), > 0 |
| Conic constant | K | 0 = sphere, −1 = paraboloid, > 0 = oblate |
| Zernike coefficients | aᵢ | 6 radial correction terms (mm), i = 0…5 |

`sag()` returns 0 when the discriminant goes negative (r outside valid aperture). For K > 0 (oblate), the aperture edge is at r = R / √(1+K).

---

## Toolpath Engine

### Polar Raster Sampling

```
r[i] = i · r_max / (n_radial − 1)       i = 0 … n_radial−1
θ[a] = a · 2π / n_angles                a = 0 … n_angles−1
z[i] = surface.sag(r[i]) × 1000         mm → μm
```

Each radial profile is spline-fitted once in the constructor. `generate()` returns the n_angles × n_radial `ToolpathPoint` array in O(n_angles × n_radial) with zero extra heap allocation per call.

### Natural Cubic Spline — Thomas Algorithm O(N)

Interior second derivatives M₁ … M_{n−1} satisfy the uniform-spacing tridiagonal system:

```
M_{i−1} + 4·M_i + M_{i+1} = (6/h²)·(z_{i+1} − 2·z_i + z_{i−1})
M_0 = M_n = 0   (natural boundary)
```

Thomas forward sweep then back substitution in O(N). Evaluation at fractional index i + t (t ∈ [0, 1)):

```
S(t) = (1−t)·z[i] + t·z[i+1]  +  (h²/6)·((a³−a)·M[i] + (b³−b)·M[i+1])
       where a = 1−t, b = t
```

`evalAt(r_mm)` computes the interval index i and fractional position t in O(1), enabling sub-sample position queries from the control loop without regenerating the full toolpath.

### Deviation Analysis

```
RMS deviation = √(Σ|z_actual[i] − z_ref[i]|² / N)   (μm)
Peak deviation = max(|z_actual[i] − z_ref[i]|)        (μm)
```

`ToolpathGenerator::analyseDeviation()` is a static method — it compares a reference `generate()` output against a measured (or noise-corrupted) actual set without holding any state.

---

## Manufacturing FSM

```
IDLE → INITIALISING → CALIBRATING → READY → ACTIVE → FAULT
                                              ↑         │
                                           pause()   fault()
                                                         │
                                              IDLE ← reset()
```

| State | Entry condition | Action |
|---|---|---|
| IDLE | Initial / after reset | No device activity |
| INITIALISING | `start()` called | Calls `init()` on all IDevice objects |
| CALIBRATING | All `init()` pass | Calls `selfTest()` on all IDevice objects |
| READY | All `selfTest()` pass | Awaiting `activate()` |
| ACTIVE | `activate()` called | 10 ms control loop; setpoints from ToolpathGenerator |
| FAULT | Any `init()` / `selfTest()` fail, or `fault()` called | Calls `shutdown()` on all devices; latches until `reset()` |

`start()` and `calibrate()` return `false` and immediately transition to FAULT if any device fails its check. `lastError()` captures the device name and failure stage for diagnostics.

---

## HAL Layer

All four devices implement `IDevice`:

```cpp
class IDevice {
public:
    virtual bool       init()     = 0;
    virtual bool       selfTest() = 0;
    virtual void       shutdown() = 0;
    virtual const char* name()   const = 0;
};
```

| Device | Behaviour |
|---|---|
| `SpindleMotor` | Linear ramp at `ramp_rate_rpm_per_s` toward target; clamps at `max_rpm` |
| `LinearAxis` | Instant positioning to target μm; `backlash_um` constant stored for real-driver compensation |
| `PressureSensor` | Returns `base_force + N(0, σ)` in Newtons; fixed seed (42) for reproducibility |
| `SafetyInterlock` | `isTripped()` when `steady_clock::now() − last_ping_ > max_interval_ms` |

In unit tests, `MockDevice` implements `IDevice` with configurable `init_ok` / `test_ok` flags — no real devices or timers required.

---

## Telemetry Logger

Lock-free single-producer single-consumer (SPSC) ring buffer. The control loop calls `push()` without any lock; the background writer thread drains the buffer to CSV every 10 ms.

```
Producer (control loop):
  t    = tail_.load(relaxed)
  next = (t + 1) % capacity
  if next == head_.load(acquire): drop frame, return false
  buf_[t] = frame
  tail_.store(next, release)

Consumer (writer thread):
  h = head_.load(relaxed)
  t = tail_.load(acquire)
  while h != t: write row, head_.store(h++, release)
```

`stop()` sets `running_ = false` and joins the writer thread — the consumer loop continues draining until the buffer is empty before exiting, so all frames pushed before `stop()` are guaranteed to appear in the CSV.

**CSV schema:**
```
ts_us,x_um,y_um,z_um,spindle_rpm,deviation_um,event_code
```

`event_code`: 0 = normal, 1 = deviation warning (> threshold), 2 = fault.

---

## Testing

### C++ — GoogleTest (35 tests)

```bash
ctest --test-dir build --output-on-failure -V
```

| Suite | n | Tests |
|---|---|---|
| `LensSurfaceTest` | 4 | ZeroSagAtCenter, SphericalApproximation (r²/2R within 1 nm), ZernikeCoefficientAffectsSag, OutsideApertureReturnsZero |
| `ToolpathTest` | 4 | PointCountMatchesGrid, RMSDeviationZeroWhenIdentical, RMSDeviationForUniformOffset, SplineEvalAtKnotIsExact |
| `FsmTest` | 11 | StartsInIdle, StartTransitionsToInitialising, CalibrateReachesReady, ActivateFromReady, PauseBackToReady, FaultFromActiveGoesFault, FaultCapturesReason, FaultCallsShutdownOnDevices, ResetFromFaultToIdle, SelfTestFailureGoesFault, StartFromFaultReturnsFalse |
| `SpindleTest` | 5 | SelfTestPassesAfterInit, RampsTowardsTarget, ReachesTargetExactly, ClampsAtMaxRpm, ShutdownZeroesRpm |
| `LinearAxisTest` | 3 | InitAndSelfTest, MovesToTarget, StartsAtZero |
| `PressureSensorTest` | 2 | ReadsNearBaseForce, SelfTestPassesAfterInit |
| `SafetyInterlockTest` | 2 | NotTrippedAfterPing, TrippedAfterTimeout |
| `TelemetryTest` | 4 | PushSucceeds, PushFailsWhenFull, DroppedCountIncrements, StopFlushesAllFrames |
