# Cross-Platform Signal Analyzer, Optical Positioning Simulator, CT Reconstruction Engine, Electrostatic Monitor, DICOM Pipeline & Optical Sorting Pipeline

Six C++ systems in one repository — sharing CMake infrastructure, GoogleTest, and a Linux + Windows CI matrix.

| Project | Description | Docs |
|---|---|---|
| **Cross-Platform Signal Analyzer** | FFT + IIR signal processing suite with abstract OOP interfaces, Qt 6 dashboard, and Python cross-validation; compiles on Linux and Windows | [docs/signal-analyzer.md](docs/signal-analyzer.md) |
| **High-Precision Optical Positioning Simulator** | Aspheric lens toolpath engine (cubic spline via Thomas algorithm), four-device HAL layer, six-state manufacturing FSM, and a lock-free SPSC telemetry logger | [docs/optical-positioning.md](docs/optical-positioning.md) |
| **CT Slice Reconstruction Engine & QML Viewer** | C++20 Filtered Back Projection engine with Ram-Lak / Shepp-Logan filters, `std::execution::par_unseq` parallelism, Qt 6/QML dark-theme viewer via `QQuickImageProvider`, and clang-tidy IEC 62304-style CI | [docs/ct-reconstruction.md](docs/ct-reconstruction.md) |
| **Electrostatic Charge Monitoring & QML Dashboard** | C++17 ADC and exponential decay simulator with a Qt6/QML Canvas gauge and QtCharts dashboard, featuring an asynchronous JSON Telemetry POST client. | [docs/electrostatic-monitor.md](docs/electrostatic-monitor.md) |
| **AI-Generated DICOM Anonymization & Observability Pipeline** | C++20 daemon featuring deterministic SHA-256 PII scrubbing, formatted NDJSON log streams, and an asynchronous HTTP server exposing Prometheus metrics. | [docs/dicom-pipeline.md](docs/dicom-pipeline.md) |
| **High-Speed Edge Inference Pipeline for Optical Sorting** | Real-time C++20 edge processing service simulating camera capture, object detection, NMS bounding box filtering, lock-free SPSC queue, and raw POSIX socket HTTP/1.1 telemetry to a FastAPI & TS dashboard. | [docs/optical-sorting.md](docs/optical-sorting.md) |

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
├── dicom_pipeline/                  AI DICOM Anonymization & Observability Pipeline
│   ├── include/                     Header definitions (Types, Reader, Anonymizer, etc.)
│   ├── src/                         Daemon C++20 source code
│   ├── tests/                       13 GoogleTests
│   ├── tests_python/                3 pytest integration tests
│   └── docker/                      Multi-stage Dockerfile
├── edge_sorting_pipeline/           High-Speed Edge Inference Pipeline for Optical Sorting
│   ├── include/                     Header definitions (postproc, inference, control, telemetry)
│   ├── src/                         Edge simulator C++20 source code
│   ├── dashboard/                   FastAPI python backend & TypeScript frontend dashboard
│   ├── tests/                       27 GoogleTests
│   ├── Dockerfile                   Multi-stage Docker build config
│   └── docker-compose.yml           Orchestrated simulation environment
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
├── k8s/                             Kubernetes ConfigMap, Deployment, and Service manifests
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
# Build all systems (including DICOM and sorting pipelines)
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_DICOM=ON -DBUILD_SORTING=ON
cmake --build build -j$(nproc)

# Run signal analyzer CLI
./build/signal_analyzer

# Run DICOM pipeline daemon CLI
./build/dicom_pipeline/dicom_anonymizer --input-dir <dir> --output-dir <dir>

# Run Optical Sorting Edge Simulator CLI
./build/edge_sorting_pipeline/edge_sorting_app

# Run all 110 GoogleTests
ctest --test-dir build --output-on-failure
```

**Windows (MSVC):**
```powershell
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_DICOM=ON
cmake --build build --config Release
.\build\Release\signal_analyzer.exe
```

---

## CI

| Job | Platform | Tests |
|---|---|---|
| `cpp-build-test` | ubuntu-latest + windows-latest | cmake build + 110 GoogleTests |
| `python-tests` | ubuntu-latest | 13 pytest (signal generator + DICOM + sorting integration tests) |
