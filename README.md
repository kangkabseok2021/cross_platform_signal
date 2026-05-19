# Cross-Platform Signal Analyzer & Optical Positioning Simulator

Two C++17 systems in one repository — sharing an abstract OOP interface architecture, bundled SQLite logging, and a Linux + Windows CI matrix.

| Project | Description | Docs |
|---|---|---|
| **Cross-Platform Signal Analyzer** | FFT + IIR signal processing suite with abstract OOP interfaces, Qt 6 dashboard, and Python cross-validation; compiles on Linux and Windows via a single `CMakeLists.txt` | [docs/signal-analyzer.md](docs/signal-analyzer.md) |
| **High-Precision Optical Positioning Simulator** | Aspheric lens toolpath engine (cubic spline via Thomas algorithm), four-device HAL layer, six-state manufacturing FSM, and a lock-free SPSC telemetry logger | [docs/optical-positioning.md](docs/optical-positioning.md) |

---

## Repository Layout

```
cross_platform_signal/
├── src/
│   ├── ISignalSource.h              Abstract source interface
│   ├── ISignalFilter.h              Abstract filter interface
│   ├── SyntheticSource.h            A·sin(2πft) + Gaussian noise
│   ├── CsvFileSource.h              Sequential CSV reader, zero-pads
│   ├── FftAnalyser.h/cpp            Cooley-Tukey radix-2 DIT FFT
│   ├── ButterworthFilter.h/cpp      4th-order IIR Direct-Form II transposed
│   ├── MovingAverageFilter.h        Sliding window, O(N)
│   ├── SqliteLogger.h/cpp           Run-metric logger (bundled SQLite)
│   ├── SignalProcessor.h/cpp        Orchestrator: source → filter → log
│   ├── main.cpp                     CLI runner
│   ├── hal/
│   │   ├── IDevice.h                Abstract HAL interface
│   │   ├── SpindleMotor.h/cpp       Spindle with linear ramp model
│   │   ├── LinearAxis.h/cpp         X/Y/Z micro-step positioning
│   │   ├── PressureSensor.h/cpp     Contact-force sensor (Gaussian noise)
│   │   └── SafetyInterlock.h/cpp    Watchdog — trips if ping() times out
│   └── optical/
│       ├── LensSurface.h            Aspheric sag formula + Zernike corrections
│       ├── ToolpathGenerator.h/cpp  Polar raster + cubic spline (Thomas O(N))
│       ├── PositioningFsm.h/cpp     6-state manufacturing FSM
│       └── TelemetryLogger.h/cpp    SPSC lock-free ring buffer → CSV
├── tests/
│   ├── test_fft.cpp                 7 GoogleTests
│   ├── test_butterworth.cpp         5 GoogleTests
│   ├── test_sources.cpp             7 GoogleTests
│   ├── test_toolpath.cpp            8 GoogleTests
│   ├── test_positioning_fsm.cpp    11 GoogleTests
│   └── test_hal.cpp                12 GoogleTests
├── tools/
│   ├── generate_signal.py           NumPy + SciPy signal + reference generator
│   └── test_tools.py               5 pytest tests
├── third_party/
│   ├── sqlite3.c                    SQLite 3.47.2 amalgamation (bundled)
│   └── sqlite3.h
├── conanfile.txt                    Conan 2: fftw/3.3.10 + gtest/1.14.0
├── CMakeLists.txt                   Single file — no platform-specific blocks
└── .github/workflows/ci.yml        ubuntu-latest + windows-latest matrix
```

---

## Quick Start

```bash
# Build both systems — no external C++ dependencies needed
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# Run signal analyzer CLI
./build/signal_analyzer

# Run all 54 GoogleTests
ctest --test-dir build --output-on-failure
```

**Windows (MSVC):**
```powershell
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
.\build\Release\signal_analyzer.exe
```

---

## CI

| Job | Platform | Tests |
|---|---|---|
| `cpp-build-test` | ubuntu-latest + windows-latest | cmake build + 54 GoogleTests |
| `python-tests` | ubuntu-latest | 5 pytest (signal generator) |
