# Cross-Platform Signal Analyzer, Optical Positioning Simulator, CT Reconstruction Engine, Electrostatic Monitor, DICOM Pipeline, Optical Sorting Pipeline, Real-Time Audio Engine, Vehicle Telemetry Qt Dashboard & Automotive ANC Engine

Nine C++ systems in one repository — sharing CMake infrastructure, GoogleTest, and a Linux + Windows CI matrix.

| Project | Description | Docs |
|---|---|---|
| **Cross-Platform Signal Analyzer** | FFT + IIR signal processing suite with abstract OOP interfaces, Qt 6 dashboard, and Python cross-validation; compiles on Linux and Windows | [docs/signal-analyzer.md](docs/signal-analyzer.md) |
| **High-Precision Optical Positioning Simulator** | Aspheric lens toolpath engine (cubic spline via Thomas algorithm), four-device HAL layer, six-state manufacturing FSM, and a lock-free SPSC telemetry logger | [docs/optical-positioning.md](docs/optical-positioning.md) |
| **CT Slice Reconstruction Engine & QML Viewer** | C++20 Filtered Back Projection engine with Ram-Lak / Shepp-Logan filters, `std::execution::par_unseq` parallelism, Qt 6/QML dark-theme viewer via `QQuickImageProvider`, and clang-tidy IEC 62304-style CI | [docs/ct-reconstruction.md](docs/ct-reconstruction.md) |
| **Electrostatic Charge Monitoring & QML Dashboard** | C++17 ADC and exponential decay simulator with a Qt6/QML Canvas gauge and QtCharts dashboard, featuring an asynchronous JSON Telemetry POST client | [docs/electrostatic-monitor.md](docs/electrostatic-monitor.md) |
| **AI-Generated DICOM Anonymization & Observability Pipeline** | C++20 daemon featuring deterministic SHA-256 PII scrubbing, formatted NDJSON log streams, and an asynchronous HTTP server exposing Prometheus metrics | [docs/dicom-pipeline.md](docs/dicom-pipeline.md) |
| **High-Speed Edge Inference Pipeline for Optical Sorting** | Real-time C++20 edge processing service simulating camera capture, object detection, NMS bounding box filtering, lock-free SPSC queue, and raw POSIX socket HTTP/1.1 telemetry to a FastAPI & TS dashboard | [docs/optical-sorting.md](docs/optical-sorting.md) |
| **Real-Time Audio Processing Engine** | C++20 lock-free SPSC ring buffer feeding a `std::jthread` IIR DSP consumer at 44.1 kHz; Qt6/QML rotary-knob UI with live Canvas waveform; SQLite preset persistence via QtSql; 13 GoogleTests + 7 QTests | [docs/realtime-audio-engine.md](docs/realtime-audio-engine.md) |
| **Embedded Vehicle Telemetry & Qt/QML Dashboard** | C++20 EMA filter bank (S_t = α·X_t + (1−α)·S_{t−1}) smoothing 4 ADC/CAN channels at 10 Hz; Qt6/QML dark-mode HMI with Canvas gauges and ChartView; httplib embedded REST server for technician browser access; 6 GoogleTests + 5 pytest REST tests | [vehicle_telemetry_qt/README.md](vehicle_telemetry_qt/README.md) |
| **Automotive Hybrid ANC Engine** | C++17 header-only FxLMS feedforward + IMC feedback active noise cancellation engine with fault detection (mic dropout, clipping, actuator degradation, divergence); Python MATLAB-equivalent design layer; 22 GoogleTests + 13 pytest | — |

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
├── realtime_audio_engine/           Real-Time Audio Processing Engine
│   ├── include/
│   │   ├── AudioConfig.h            constexpr constants + Diagnostics struct
│   │   ├── AudioRingBuffer.h        SPSC lock-free ring (alignas(64) false-sharing fix)
│   │   └── IirFilter.h              y[n] = α·x[n] + (1-α)·y[n-1], atomic cutoff
│   ├── src/
│   │   ├── AudioEngine.h/cpp        jthread producer (44.1 kHz) + DSP consumer
│   │   ├── AudioEngineWrapper.h/cpp Q_PROPERTY bridge + 100 ms poll timer
│   │   ├── PresetRepository.h/cpp   QtSql/SQLite (UNIQUE + CHECK constraints, WAL)
│   │   ├── PresetModel.h/cpp        QAbstractListModel for QML ListView
│   │   └── main.cpp                 Qt application entry point
│   ├── qml/
│   │   ├── RotaryKnob.qml           270° Canvas arc, vertical-drag interaction
│   │   ├── WaveformView.qml         512-sample Canvas oscilloscope (16 ms repaint)
│   │   └── main.qml                 Dark-themed ApplicationWindow
│   ├── tests/
│   │   ├── test_audio_core.cpp      13 GoogleTests (IirFilter×5, RingBuffer×5, Engine×3)
│   │   └── test_audio_qt.cpp        7 QTests (PresetRepository×4, Wrapper×3)
│   └── docs/
│       ├── THREADING.md             Lock-free rationale + false-sharing analysis
│       ├── FILTER-MATH.md           IIR derivation, stability proof, IIR vs FIR table
│       └── DB-SCHEMA.md             SQLite schema DDL + ADR-DB-001
├── vehicle_telemetry_qt/            Embedded Vehicle Telemetry & Qt/QML Dashboard
│   ├── include/
│   │   ├── EmaFilter.h              Header-only EmaFilter + EmaFilterBank<N> (S_t = α·X_t + (1−α)·S_{t−1})
│   │   ├── SensorSimulator.h        VehicleSensorSimulator — 4 channels, mt19937 noise
│   │   ├── TelemetryHttpServer.h    httplib pimpl REST server (GetSnapshot/SetAlpha callbacks)
│   │   └── TelemetryModel.h         Qt QObject — Q_PROPERTY/Q_INVOKABLE/NOTIFY bridge
│   ├── src/
│   │   ├── SensorSimulator.cpp      engine_temp · battery_v · oil_pressure · rpm
│   │   ├── TelemetryHttpServer.cpp  httplib + nlohmann/json routes + embedded index.html
│   │   ├── TelemetryModel.cpp       QTimer 10 Hz → sim → EmaFilterBank → emit dataChanged
│   │   ├── main_noqt.cpp            Headless server (--no-qt, std::thread loop, for pytest)
│   │   └── main.cpp                 Qt app entry (context property + QML engine)
│   ├── qml/
│   │   ├── main.qml                 ApplicationWindow 1024×600, fault badge, alpha sliders
│   │   ├── GaugeCard.qml            Canvas 270° arc gauge, fault color animation
│   │   └── TelemetryChart.qml       ChartView raw/filtered LineSeries, 60 s rolling window
│   ├── tests/test_ema.cpp           6 GoogleTests (passthrough, smoothing, step-response, reset, steady-state, spike)
│   ├── tests_python/test_http_server.py  5 pytest REST tests (health, 4 channels, EMA diff, alpha, CORS)
│   ├── CMakeLists.txt               vt_ema_core + vt_sensor_core + test_ema + optional HTTP + Qt
│   ├── Jenkinsfile                  4-stage: deps → EMA tests → headless HTTP → pytest → Qt GUI
│   └── README.md
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
├── automotive_anc_engine/           Hybrid ANC Engine — FxLMS Feedforward + IMC Feedback
│   ├── include/anc/
│   │   ├── secondary_path.hpp       FIR secondary-path model (circular-buffer convolution)
│   │   ├── fxlms.hpp                FxLMS adaptive filter (leakage, freeze/unfreeze)
│   │   ├── imc.hpp                  IMC feedback controller (Hann-windowed FIR low-pass)
│   │   ├── fault_detector.hpp       Fault detection FSM (dropout/clip/divergence/degradation)
│   │   └── hybrid_anc.hpp           Hybrid controller combining FxLMS + IMC + FaultDetector
│   ├── tests/                       22 GoogleTests (6 FxLMS, 4 IMC, 5 Fault, 4 Hybrid, 3 Parity, 4 Regression)
│   ├── design/                      Python MATLAB-equivalent design + simulation layer
│   │   ├── anc/                     CabinNoiseGenerator, SecondaryPath, FxLMS, IMCController
│   │   └── tests/                   13 pytest tests
│   └── CMakeLists.txt               Standalone subproject (FetchContent GTest guard)
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
# Build core systems (no Qt required)
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_DICOM=ON -DBUILD_SORTING=ON
cmake --build build -j$(nproc)

# Build audio engine (requires Qt6)
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_AUDIO=ON
cmake --build build --target audio_app -j$(nproc)

# Run signal analyzer CLI
./build/signal_analyzer

# Run DICOM pipeline daemon CLI
./build/dicom_pipeline/dicom_anonymizer --input-dir <dir> --output-dir <dir>

# Run Optical Sorting Edge Simulator CLI
./build/edge_sorting_pipeline/edge_sorting_app

# Run all GoogleTests (core + audio)
ctest --test-dir build --output-on-failure

# Run audio Qt tests (offscreen)
QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure -R "test_audio_qt"
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
| `audio-test` | ubuntu-24.04 (Qt6) | 13 GoogleTests + 7 QTests (offscreen) |
| `ct-backend-test` | ubuntu-latest | 6 GoogleTests (CT FilterBank + Reconstructor) |
| `clang-tidy` | ubuntu-latest | cppcoreguidelines + modernize checks on ct_core |
| `automotive-anc-engine-test` | ubuntu-latest | 22 GoogleTests (FxLMS, IMC, HybridANC, FaultDetector, parity, regression) + 13 pytest (Python design layer) |
| `python-tests` | ubuntu-latest | 13 pytest (signal generator + DICOM + sorting integration tests) |

---

## Automotive Hybrid ANC Engine

C++17 header-only active noise cancellation engine for automotive cabin use, with a Python simulation layer mirroring the MATLAB design workflow.

### Architecture

```
Reference mic (tachometer)          Error mic
        │                                │
        ▼                                │
  ┌──────────┐   anti-noise y(n)         │
  │  FxLMS   │──────────────────────────►│─► e(n) = d(n) − y_s(n)
  │feedforward│◄──── filt_ref x̂(n) ─────│
  └──────────┘     (secondary path)      │
        │                                │
        │             ┌──────────┐       │
        │             │   IMC    │◄──────┘
        │             │feedback  │
        │             └────┬─────┘
        │                  │  u(n)
        └──────────────────┤
                           ▼
                    y(n) = ff(n) + fb(n)
```

**FxLMS** (`fxlms.hpp`): Leaky filtered-x LMS with configurable filter length and secondary-path model.
Update rule: `w(n+1) = (1−λ)·w(n) + μ·x̂(n)·e(n)` where `x̂(n)` is the reference filtered through the secondary-path estimate.

**IMC** (`imc.hpp`): Hann-windowed FIR low-pass approximating `Q(z) = Ŝ⁻¹(z)·F(z)` for broadband residual suppression.

**FaultDetector** (`fault_detector.hpp`): Four-fault FSM — mic dropout (`|e| < τ_drop`), mic clipping (`|e| > τ_clip`), actuator degradation (weight-norm decay), divergence (norm explosion). Transitions ANC state between `ACTIVE`, `ADAPTATION_FROZEN`, `MUTED`, `PASSIVE`.

### Quick Start

```bash
# Build and run all 22 GoogleTests
cmake -S automotive_anc_engine -B build-anc -DCMAKE_BUILD_TYPE=Release
cmake --build build-anc --parallel
ctest --test-dir build-anc --output-on-failure -V

# Run Python design-layer simulation (13 pytest)
cd automotive_anc_engine/design
uv run pytest tests/ -v
```

### Test Coverage

| Suite | Tests | What is verified |
|---|---|---|
| `test_fxlms` | 6 | Weight update, zero-μ, leakage bound, closed-loop convergence, energy bound, reset |
| `test_imc` | 4 | Zero-input, broadband attenuation, bounded input, reset |
| `test_fault` | 5 | Dropout, clipping, divergence detection, normal-active state, clear-fault |
| `test_hybrid_anc` | 4 | Near-zero init, hybrid error reduction, reset, muted state |
| `test_parity` | 3 | FxLMS output vs golden, final weights vs golden, secondary-path convolution |
| `test_regression` | 4 | Low/high RPM finite output, fault injection safety, noise reduction after convergence |
