# Embedded Vehicle Telemetry & Qt/QML Dashboard

C++20 embedded maintenance terminal: four ADC/CAN sensor channels smoothed by recursive EMA filters, rendered on a dark-mode Qt6/QML touchscreen HMI, with an embedded httplib REST server for off-screen technician access.

## Architecture

```
VehicleSensorSimulator (QTimer 10 Hz, 4 ch)
        │  engine_temp · battery_v · oil_pressure · rpm
        ▼
  EmaFilterBank<4>   S_t = α·X_t + (1−α)·S_{t−1}
        │  filtered values
        ▼
  TelemetryModel (QObject / Q_PROPERTY / NOTIFY)
   ├── QML Dashboard ─── Canvas gauges + ChartView
   └── TelemetryHttpServer ─── background std::thread
            GET /api/v1/telemetry → JSON
            POST /api/v1/channels/{id}/alpha → retune EMA
```

## EMA Filter

| Property | Value |
|---|---|
| Formula | `S_t = α·X_t + (1−α)·S_{t−1}` |
| Alpha range | (0, 1] — clamped at construction |
| `α = 1.0` | passthrough (no smoothing) |
| `α = 0.1` | strong smoothing (~10 tick time constant) |
| reset(init) | restarts state — cold-start without transient |

Step-response example (α=0.5, input step 0→100):

| Step | Output |
|---|---|
| 1 | 50.0 |
| 2 | 75.0 |
| 3 | 87.5 |
| 4 | 93.75 |
| 5 | **96.875** (≥ 95 — test threshold) |

## Sensor Channels

| # | Name | Unit | Baseline | σ (noise) | Fault threshold |
|---|---|---|---|---|---|
| 0 | engine_temp | °C | 80 | 3.0 | > 115 °C |
| 1 | battery_v | V | 12.8 | 0.15 | < 11.8 V |
| 2 | oil_pressure | bar | 3.5 | 0.2 | < 2.2 bar |
| 3 | rpm | RPM | 2000 | 30 | > 4500 RPM |

## Quick Start

```bash
# Build EMA tests only (no Qt, no httplib)
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_VT_HTTP=OFF \
      -DCMAKE_POLICY_DEFAULT_CMP0135=NEW
cmake --build build --target test_ema -j$(nproc)
ctest --test-dir build -V

# Build headless server + run pytest
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_VT_HTTP=ON \
      -DCMAKE_POLICY_DEFAULT_CMP0135=NEW
cmake --build build --target vehicle_telemetry_headless -j$(nproc)
TELEMETRY_BINARY=build/vehicle_telemetry_headless pytest tests_python/ -v

# Build Qt dashboard (requires Qt6)
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_VT_HTTP=ON
cmake --build build --target vehicle_telemetry -j$(nproc)
./build/vehicle_telemetry

# Access technician web UI
open http://localhost:8080
```

## Tests

### C++ — 6 GoogleTests (`test_ema`, no Qt)

| Test | What it verifies |
|---|---|
| `AlphaOnePassthrough` | α=1.0 → process(x) == x for all x |
| `SmallAlphaStrongSmoothing` | α≈0 → output change < 1% of step per tick |
| `StepResponseConvergence` | α=0.5, step 0→100 → ≥95 within 5 steps |
| `ResetAndConstantInput` | reset(50.0) + process(50.0) == 50.0 |
| `ConstantInputSteadyState` | constant input C → steady-state = C |
| `FaultSpikeRejection` | ×10 spike at α=0.1 → smoothed < 3× baseline |

### Python — 5 pytest REST integration tests

| Test | What it verifies |
|---|---|
| `test_health_ok` | GET /api/v1/health → 200 + `{"status":"ok","uptime_s":N}` |
| `test_telemetry_has_four_channels` | GET /api/v1/telemetry → 4 channel objects with name/raw/filtered/alpha/fault |
| `test_filtered_differs_from_raw` | After 1 s, EMA has diverged from raw noise |
| `test_set_alpha` | POST /api/v1/channels/0/alpha `{"alpha":0.9}` → confirmed via GET |
| `test_cors_header` | Access-Control-Allow-Origin: * on all responses |
