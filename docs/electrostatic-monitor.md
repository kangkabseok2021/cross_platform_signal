# Electrostatic Charge Monitoring & QML Dashboard

## Architecture
- **C++ Engine**: Simulates a 16-bit ADC reading static charge from an environment. Implements an exponential charge decay model `Q(t) = Q₀·e^(−t/τ)`.
- **QML Dashboard**: Renders a live `Canvas`-based gauge and a `QtCharts::ChartView` to visualize the discharge curve in real time.
- **Telemetry Client**: Uses `QNetworkAccessManager` to POST JSON telemetry to a cloud endpoint when a discharge event occurs.

## Physics
- **Charge Decay**: When the charge reaches 2.0 kV, an ionizer is deployed, dropping the relaxation constant `τ` from 50.0s (ABS plastic) to 0.5s (ionized air path), causing rapid discharge.
- **Energy Calculation**: `E = ½CV²`

## Running
Requires Qt 6.5+ and CMake.
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
./build/electrostatic_app
```
