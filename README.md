# Cross-Platform Signal Analysis Interface

A C++17 signal processing suite that ingests simulated sensor data, applies FFT analysis and IIR digital filtering through abstract OOP interfaces, and logs metrics to SQLite. A Qt 6 dashboard visualises raw vs. filtered signals in real time. The same `CMakeLists.txt` compiles without modification on Linux (GCC) and Windows (MSVC). GitHub Actions CI runs on both platforms on every commit. A Python tool generates test signals and provides SciPy-computed reference values for cross-language numerical validation.

---

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│  Python  tools/generate_signal.py                               │
│  NumPy sine + noise  →  data/signal.csv                         │
│  SciPy reference FFT →  data/reference.json  (GoogleTest input) │
└────────────────────┬────────────────────────────────────────────┘
                     │ CSV
┌────────────────────▼────────────────────────────────────────────┐
│  ISignalSource  (abstract)                                      │
│  ├── SyntheticSource   A·sin(2πft) + Gaussian noise             │
│  └── CsvFileSource     sequential CSV reader, zero-pads         │
└────────────────────┬────────────────────────────────────────────┘
                     │ std::vector<double>
┌────────────────────▼────────────────────────────────────────────┐
│  ISignalFilter  (abstract)                                      │
│  ├── FftAnalyser       Cooley-Tukey radix-2 DIT, O(N log N)     │
│  ├── ButterworthFilter 4th-order IIR Direct-Form II, O(N)       │
│  └── MovingAverageFilter sliding window, O(N)                   │
└────────────────────┬────────────────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────────────────────┐
│  SignalProcessor                                                │
│  source.read(N) → filter.apply() → fft.analyse() → log         │
└──────┬─────────────────────────────────────┬────────────────────┘
       │                                     │
┌──────▼──────────────────┐     ┌────────────▼────────────────────┐
│  Qt 6 Dashboard         │     │  SqliteLogger                   │
│  Raw + filtered charts  │     │  signal_runs (bundled SQLite)   │
│  FFT spectrum view      │     │  run_id · ts · rms · latency_us │
└─────────────────────────┘     └────────────────────────────────┘
```

---

## Signal Processing

### FFT Analyser — O(N log N)

Cooley-Tukey radix-2 DIT (decimation-in-time) FFT in pure C++. Zero-pads input to the next power-of-2 automatically.

```
Frequency bin resolution = sample_rate / N   (Nyquist)
Output: |X[k]| / N  for k = 0 .. N/2   (N/2+1 values)
```

In production: swap `FftAnalyser.cpp` to call `fftw_plan_dft_r2c_1d` via Conan (`conanfile.txt` declares `fftw/3.3.10`).

### Butterworth Low-Pass — O(N)

4th-order Butterworth implemented as two cascaded 2nd-order sections (biquads) in **Direct-Form II transposed** — the numerically stable canonical form. Coefficients computed analytically at construction via bilinear transform from the s-domain prototype:

```
H(s) = 1 / [(s/ωc)⁴ + 2.613(s/ωc)³ + 3.414(s/ωc)² + 2.613(s/ωc) + 1]
```

No lookup table — valid for any cutoff frequency and sample rate.

### Moving Average — O(N)

Sliding window with running sum: O(1) per sample, state preserved across calls.

---

## Project Structure

```
cross_platform_signal/
├── src/
│   ├── ISignalSource.h          Abstract source interface
│   ├── ISignalFilter.h          Abstract filter interface
│   ├── SyntheticSource.h        A·sin(2πft) + Gaussian noise
│   ├── CsvFileSource.h          Sequential CSV reader
│   ├── FftAnalyser.h/cpp        Cooley-Tukey FFT + spectrum analysis
│   ├── ButterworthFilter.h/cpp  4th-order IIR biquad cascade
│   ├── MovingAverageFilter.h    Sliding window filter
│   ├── SqliteLogger.h/cpp       Run metrics logger
│   ├── SignalProcessor.h/cpp    Orchestrator: source → filter → log
│   └── main.cpp                 CLI runner
├── tests/
│   ├── test_fft.cpp             7 GoogleTests
│   ├── test_butterworth.cpp     5 GoogleTests
│   └── test_sources.cpp        7 GoogleTests
├── tools/
│   ├── generate_signal.py       NumPy + SciPy signal generator
│   └── test_tools.py           5 pytest tests
├── third_party/
│   ├── sqlite3.c               SQLite 3.47.2 amalgamation
│   └── sqlite3.h               (bundled — no system SQLite required)
├── conanfile.txt               Conan 2: fftw/3.3.10 + gtest/1.14.0
├── CMakeLists.txt              Single file — no platform-specific blocks
└── .github/workflows/ci.yml   ubuntu-latest + windows-latest matrix
```

---

## Quick Start

### Build (Linux / macOS)

```bash
# No external C++ dependencies needed — SQLite is bundled
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
./build/signal_analyzer
```

### Build (Windows — MSVC)

```powershell
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
.\build\Release\signal_analyzer.exe
```

No platform-specific blocks in `CMakeLists.txt`. Works identically on Linux, macOS, and Windows.

### Build with Qt 6 Dashboard

```bash
cmake -B build -DBUILD_GUI=ON
cmake --build build -j$(nproc)
./build/signal_dashboard
```

Requires Qt 6 with `Widgets` and `Charts` modules.

### CLI Output

```
Source:         synthetic 50 Hz + noise
Filter:         Butterworth low-pass 150 Hz
Samples:        1024
RMS raw:        0.714
RMS filtered:   0.706
Fundamental Hz: 49.805
Latency µs:     312
```

---

## Python Signal Generator

```bash
uv sync
uv run python tools/generate_signal.py --freq 50 --snr 20 --n 1024
# → data/signal.csv       consumed by CsvFileSource
# → data/reference.json   SciPy FFT reference for GoogleTest validation
```

| Flag | Default | Description |
|---|---|---|
| `--freq` | 50.0 | Signal frequency (Hz) |
| `--amp` | 1.0 | Amplitude |
| `--snr` | 20.0 | SNR (dB) |
| `--sr` | 1000.0 | Sample rate (Hz) |
| `--n` | 1024 | Number of samples |
| `--seed` | 42 | RNG seed for reproducibility |

---

## Testing

### C++ — GoogleTest (19/19)

```bash
ctest --test-dir build --output-on-failure -V
```

| Suite | n | What's validated |
|---|---|---|
| `FftTest` | 7 | Fundamental ±1 bin, magnitude > 0, output length N/2+1, bin resolution = SR/N, DC bin, zero-pad non-pow2, two-tone peak separation |
| `ButterworthTest` | 5 | Passband RMS ≈ 1/√2, stopband < 0.1 at 4× cutoff, output length == input, name string, attenuation at cutoff |
| `SyntheticSourceTest` | 5 | Count, sample rate, t=0 is zero, amplitude peak, sequential continuity |
| `CsvFileSourceTest` | 2 | Reads correct values, zero-pads on file exhaustion |

### Python — pytest (5/5)

```bash
uv run pytest tools/ -v
```

CSV line count, reference JSON schema, frequency accuracy ±2 bins, float parseability, seed reproducibility.

---

## CI/CD Pipeline

Two jobs on every push to `main`:

```
cpp-build-test  [ubuntu-latest]   cmake -B build && cmake --build && ctest
                [windows-latest]  same — no system SQLite install needed
                                  (SQLite bundled via amalgamation)

python-tests    [ubuntu-latest]   uv sync --frozen && pytest tools/
```

**Cross-platform design note:** `find_package(SQLite3)` was replaced with a bundled `sqlite3_bundled` static library target (`third_party/sqlite3.c`). This removes all external C++ dependencies — the project builds with only a compiler and CMake on any platform.

---

## Conan 2 Integration

`conanfile.txt` declares `fftw/3.3.10` and `gtest/1.14.0` for production Conan-managed builds:

```bash
pip install conan
conan profile detect
conan install . --output-folder=build --build=missing
cmake -B build --preset conan-release
cmake --build build
```

Replaces the Cooley-Tukey implementation with FFTW3 for maximum throughput and adds the FFTW wisdom file for deterministic startup latency.

---

## SQLite Schema

```sql
CREATE TABLE signal_runs (
    run_id         INTEGER PRIMARY KEY AUTOINCREMENT,
    ts             TEXT    NOT NULL,     -- ISO 8601 UTC
    source_type    TEXT    NOT NULL,     -- "synthetic" | "csv"
    filter_type    TEXT    NOT NULL,     -- "FFT" | "Butterworth" | "MovingAverage"
    sample_count   INTEGER NOT NULL,
    fundamental_hz REAL,                -- dominant frequency (FFT peak bin)
    rms_raw        REAL,                -- RMS of unfiltered signal
    rms_filtered   REAL,                -- RMS after filter
    latency_us     INTEGER              -- end-to-end processing time µs
);
```

Enables trend analysis across runs: filter latency regression, fundamental frequency stability, RMS attenuation ratio per filter type.
